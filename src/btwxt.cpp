/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include <iostream>
#include <numeric>

//btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {


    RegularGridInterpolator::RegularGridInterpolator() {};

    RegularGridInterpolator::RegularGridInterpolator(GriddedData &the_blob) :
            the_blob(the_blob),
            current_grid_point(),  // instantiates an empty GridPoint
            cgp_exists(false) {
        std::size_t ndims{get_ndims()};
        core_hypercube = CoreHypercube(ndims);
        showMessage(MSG_DEBUG, "RGI constructed from GriddedData!");
    };

    RegularGridInterpolator::RegularGridInterpolator(
            const std::vector<std::vector<double> > &grid,
            const std::vector<std::vector<double> > &values
    ) :
            the_blob(grid, values),
            current_grid_point(),  // instantiates an empty GridPoint
            cgp_exists(false) {
        std::size_t ndims{get_ndims()};
        core_hypercube = CoreHypercube(ndims);
        showMessage(MSG_DEBUG, "RGI constructed from vectors!");
    };

    double RegularGridInterpolator::calculate_value_at_target(
            std::vector<double> target, std::size_t table_index) {
        set_new_grid_point(target);
        std::vector<double> result = interpolation_wrapper();
        return result[table_index];
    };

    double RegularGridInterpolator::calculate_value_at_target(
            std::size_t table_index) {
        if (cgp_exists) {
            std::vector<double> result = interpolation_wrapper();
            return result[table_index];
        }
        showMessage(MSG_WARN, "No target has been defined!");
        return 0;
    };

    std::vector<double> RegularGridInterpolator::calculate_all_values_at_target(
            std::vector<double> target) {
        set_new_grid_point(target);
        std::vector<double> result = interpolation_wrapper();
        return result;
    };

    std::vector<double> RegularGridInterpolator::calculate_all_values_at_target() {
        if (cgp_exists) {
            std::vector<double> result = interpolation_wrapper();
            return result;
        }
        showMessage(MSG_WARN, "No target has been defined!");
        return {0};
    };

    void RegularGridInterpolator::set_new_grid_point(
            const std::vector<double> &target) {
        RegularGridInterpolator::check_target_dimensions(target);
        current_grid_point = GridPoint(target);
        cgp_exists = true;
        the_locator = WhereInTheGridIsThisPoint(current_grid_point, the_blob);
    };

    std::vector<double> RegularGridInterpolator::get_current_grid_point() {
        if (cgp_exists) {
            return current_grid_point.target;
        }
        showMessage(MSG_WARN, "No target has been defined!");
        return {0};
    }

    void RegularGridInterpolator::clear_current_grid_point() {
        current_grid_point = GridPoint();
        cgp_exists = false;
        the_locator = WhereInTheGridIsThisPoint();
    };

    std::size_t RegularGridInterpolator::get_ndims() { return the_blob.get_ndims(); };

    void RegularGridInterpolator::check_target_dimensions(
            const std::vector<double> &target) {
        std::size_t ndims = the_blob.get_ndims();
        if (ndims == target.size()) {
            showMessage(MSG_DEBUG, stringify(
                    "Target and GridSpace dimensions match: ", target.size()));
        } else {
            showMessage(MSG_ERR, stringify(
                    "number of dimensions (", ndims,
                    ") does not match length of target (", target.size(), ")."));
        }
    };

    std::vector<std::size_t> RegularGridInterpolator::get_current_floor() {
        if (cgp_exists) {
            return the_locator.get_floor();
        }
        showMessage(MSG_WARN, "No target has been defined!");
        return {0};
    }

    std::vector<double> RegularGridInterpolator::get_current_weights() {
        if (cgp_exists) {
            return the_locator.get_weights();
        }
        showMessage(MSG_WARN, "No target has been defined!");
        return {0};
    }

    std::vector<std::vector<double> > RegularGridInterpolator::get_interp_coeffs() {
        return the_locator.get_interp_coeffs();
    }

    std::vector<std::vector<double> > RegularGridInterpolator::get_cubic_slope_coeffs() {
        return the_locator.get_cubic_slope_coeffs();
    }


    std::vector<double> RegularGridInterpolator::interpolation_wrapper() {
        std::size_t ndims = get_ndims();
        std::vector<int> methods = the_locator.get_methods();

        core_hypercube.collect_things(the_locator);
        Eigen::ArrayXd result = core_hypercube.compute_core(the_blob);

        // for each cubic-interp axis, get second-order contribution from the rectangle
        Eigen::ArrayXd dim_slope_weight = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
        for (std::size_t dim = 0; dim < ndims; dim++) {
            if (methods[dim] == CUBIC) {
                dim_slope_weight = core_hypercube.compute_slopes_rectangle(dim, the_blob);
                result += dim_slope_weight;
            }
        }

        // only if there are multiple cubic dimensions,
        // for each point in full_hypercube, add third-order contribution.
        if (std::count(methods.begin(), methods.end(), CUBIC) >= 2) {
            FullHypercube full_hypercube(ndims, methods);
            full_hypercube.collect_things(the_locator);
            Eigen::ArrayXd third_order_total = full_hypercube.third_order_contributions(the_blob);
            result += third_order_total;
        }

        showMessage(MSG_DEBUG, stringify("results\n", result));
        return eigen_to_vector(result);
    }


// free functions
    std::size_t index_below_in_vector(double target, std::vector<double> &my_vec)
    // returns the index of the largest value <= the target
    // if target is greater than all values, returns index of final value
    // if target is less than all values, returns index of final value +1
    {
        std::vector<double>::iterator upper;
        upper = std::upper_bound(my_vec.begin(), my_vec.end(), target);
        if ((upper - my_vec.begin()) == 0) {
            return my_vec.size();
        }
        return upper - my_vec.begin() - 1;
    }

    double compute_fraction(double x, double edge[2]) {
        // how far along an edge is the target?
        return (x - edge[0]) / (edge[1] - edge[0]);
    }

    std::size_t pow(const std::size_t &base, const std::size_t &power) {
        // raise base to a power (both must be size_t)
        if (power == 0) { return 1; }
        else {
            std::size_t result = base;
            for (std::size_t i = 1; i < power; i++) {
                result = result * base;
            }
            return result;
        }
    }


}
