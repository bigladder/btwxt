/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include <iostream>
#include <numeric>

//btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {


    RegularGridInterpolator::RegularGridInterpolator() = default;

    RegularGridInterpolator::RegularGridInterpolator(GriddedData &grid_data_in) :
            grid_data(grid_data_in),
            grid_point(grid_data)
    {}

    RegularGridInterpolator::RegularGridInterpolator(
            const std::vector<std::vector<double> > &grid,
            const std::vector<std::vector<double> > &values
    ) :
            grid_data(grid, values),
            grid_point(grid_data)
    {}

    RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source) {
        *this = source;
    }



    double RegularGridInterpolator::get_value_at_target(
            std::vector<double> target, std::size_t table_index) {
        set_new_target(target);
        std::vector<double> result = interpolation_wrapper();
        return result[table_index];
    }

    double RegularGridInterpolator::get_value_at_target(
            std::size_t table_index) {
        std::vector<double> result = interpolation_wrapper();
        return result[table_index];
    }

    std::vector<double> RegularGridInterpolator::get_values_at_target(
            std::vector<double> target) {
        set_new_target(target);
        return interpolation_wrapper();
    }

    std::vector<double> RegularGridInterpolator::get_values_at_target() {
        return interpolation_wrapper();
    }

    void RegularGridInterpolator::set_new_target(
            const std::vector<double> &target) {
        grid_point.set_target(target);
    }

    std::vector<double> RegularGridInterpolator::get_current_target() {
        return grid_point.get_current_target();
    }

    void RegularGridInterpolator::clear_current_target() {
        grid_point = GridPoint(grid_data);
    }

    std::size_t RegularGridInterpolator::get_ndims() { return grid_data.get_ndims(); }

    std::pair<double, double> RegularGridInterpolator::get_axis_limits(int dim) {
        return grid_data.get_extrap_limits(dim);
    }

    std::vector<double> RegularGridInterpolator::interpolation_wrapper() {
        return grid_point.get_results();
    }
}
