/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

// Standard
#include <functional>
#include <map>
#include <memory>
#include <vector>

// vendor
#include <courier/courier.h>
#include <courier/helpers.h>

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

enum class Method { undefined, constant, linear, cubic };

class RegularGridInterpolatorImplementation : public Courier::Sender {
    friend class GridAxis;

  public:
    RegularGridInterpolatorImplementation() = default;

    RegularGridInterpolatorImplementation(const std::vector<GridAxis>& grid_axes,
                                          std::string name,
                                          const std::shared_ptr<Courier::Courier>& courier);

    RegularGridInterpolatorImplementation(const std::vector<GridAxis>& grid_axes,
                                          const std::vector<GridPointDataSet>& grid_point_data_sets,
                                          std::string name,
                                          const std::shared_ptr<Courier::Courier>& courier);

    RegularGridInterpolatorImplementation(const RegularGridInterpolatorImplementation& source);

    RegularGridInterpolatorImplementation&
    operator=(const RegularGridInterpolatorImplementation& source) = default;

    // Data manipulation and settings
    std::size_t add_grid_point_data_set(const GridPointDataSet& grid_point_data_set);

    void set_axis_interpolation_method(std::size_t axis_index, InterpolationMethod method)
    {
        check_axis_index(axis_index, "set axis interpolation method");
        grid_axes[axis_index].set_interpolation_method(method);
    }

    void set_axis_extrapolation_method(const std::size_t axis_index, ExtrapolationMethod method)
    {
        check_axis_index(axis_index, "set axis extrapolation method");
        grid_axes[axis_index].set_extrapolation_method(method);
    }

    void set_axis_extrapolation_limits(const std::size_t axis_index,
                                       const std::pair<double, double>& limits)
    {
        check_axis_index(axis_index, "set axis extrapolation limits");
        grid_axes[axis_index].set_extrapolation_limits(limits);
    }

    // Public methods (mirrored)
    void set_target(const std::vector<double>& target);

    [[nodiscard]] const std::vector<double>& get_target() const;

    void clear_target();

    [[nodiscard]] std::vector<double> get_results() const;

    std::vector<double> get_results(const std::vector<double>& target);

    void normalize_grid_point_data_sets_at_target(double scalar = 1.0);

    double normalize_grid_point_data_set_at_target(std::size_t data_set_index, double scalar = 1.0);

    void normalize_grid_point_data_set(std::size_t data_set_index, double scalar = 1.0);

    std::string write_data();

    void set_courier(const std::shared_ptr<Courier::Courier>& courier,
                     bool set_grid_axes_couriers = false);

    // Public getters
    [[nodiscard]] std::pair<double, double> get_extrapolation_limits(std::size_t axis_index) const
    {
        check_axis_index(axis_index, "get extrapolation limits");
        return grid_axes[axis_index].get_extrapolation_limits();
    };

    [[nodiscard]] inline std::shared_ptr<Courier::Courier> get_courier() const { return courier; };

    [[nodiscard]] inline std::size_t get_number_of_grid_axes() const
    {
        return number_of_grid_axes;
    };

    [[nodiscard]] inline std::size_t get_number_of_grid_points() const
    {
        return number_of_grid_points;
    };

    [[nodiscard]] inline const GridAxis& get_grid_axis(std::size_t axis_index) const
    {
        check_axis_index(axis_index, "get grid axis");
        return grid_axes[axis_index];
    };

    [[nodiscard]] inline const GridPointDataSet&
    get_grid_point_data_set(std::size_t data_set_index) const
    {
        check_data_set_index(data_set_index, "get grid point data set");
        return grid_point_data_sets[data_set_index];
    };

    [[nodiscard]] inline std::size_t get_number_of_grid_point_data_sets() const
    {
        return number_of_grid_point_data_sets;
    };

    [[nodiscard]] inline const std::vector<std::size_t>& get_grid_axis_lengths() const
    {
        return grid_axis_lengths;
    };

    [[nodiscard]] inline const std::vector<TargetBoundsStatus>& get_target_bounds_status() const
    {
        return target_bounds_status;
    };

    [[nodiscard]] inline const std::vector<double>& get_floor_to_ceiling_fractions() const
    {
        return floor_to_ceiling_fractions;
    };

    [[nodiscard]] inline const std::vector<std::size_t>& get_floor_grid_point_coordinates() const
    {
        return floor_grid_point_coordinates;
    };

    std::vector<std::size_t> get_neighboring_indices_at_target();

    std::vector<std::size_t> get_neighboring_indices_at_target(const std::vector<double>& target);

    [[nodiscard]] inline const std::vector<std::vector<double>>&
    get_interpolation_coefficients() const
    {
        return interpolation_coefficients;
    };

    [[nodiscard]] inline const std::vector<std::vector<double>>&
    get_cubic_slope_coefficients() const
    {
        return cubic_slope_coefficients;
    };

    [[nodiscard]] inline const std::vector<Method>& get_current_methods() const { return methods; };

    [[nodiscard]] inline const std::vector<std::vector<short>>& get_hypercube()
    {
        consolidate_methods();
        return hypercube;
    };

    [[nodiscard]] const std::vector<double>&
    get_axis_cubic_spacing_ratios(std::size_t axis_index, std::size_t floor_or_ceiling) const
    {
        check_axis_index(axis_index, "get axis cubic spacing ratios");
        return grid_axes[axis_index].get_cubic_spacing_ratios(floor_or_ceiling);
    };

    const std::vector<double>& get_grid_point_data(const std::vector<std::size_t>& coordinates);
    const std::vector<double>& get_grid_point_data(std::size_t grid_point_index);

    std::vector<double> get_grid_point_data_relative(const std::vector<std::size_t>& coordinates,
                                                     const std::vector<short>& translation);

    [[nodiscard]] std::vector<Method> get_interpolation_methods() const;

    [[nodiscard]] std::vector<Method> get_extrapolation_methods() const;

    [[nodiscard]] std::size_t
    get_grid_point_index(const std::vector<std::size_t>& coordinates) const;

    double get_grid_point_weighting_factor(const std::vector<short>& hypercube_indices);

  private:
    // Structured data
    std::vector<GridAxis> grid_axes;
    std::vector<GridPointDataSet> grid_point_data_sets;
    std::size_t number_of_grid_points {0u};
    std::size_t number_of_grid_point_data_sets {0u};
    std::size_t number_of_grid_axes {0u};
    std::vector<std::size_t>
        grid_axis_lengths; // Number of points in each grid axis (size = number_of_grid_axes)
    std::vector<std::size_t> grid_axis_step_size;   // Used to translate grid point coordinates to
                                                    // indices (size = number_of_grid_axes)
    std::vector<std::size_t> temporary_coordinates; // Memory placeholder to avoid re-allocating
                                                    // memory (size = number_of_grid_axes)
    std::vector<double> temporary_grid_point_data;  // Pre-sized container to store set of data at
                                                    // grid point coordinates

    // calculated data
    bool target_is_set {false};
    std::vector<double> target;
    std::vector<std::size_t>
        floor_grid_point_coordinates; // coordinates of the grid point <= target
    std::size_t floor_grid_point_index {
        0u}; // Index of the floor_grid_point_coordinates (used for hypercube caching)
    std::vector<double> floor_to_ceiling_fractions; // for each axis, the fraction the target value
                                                    // is between its floor and ceiling axis values
    std::vector<TargetBoundsStatus>
        target_bounds_status; // for each axis, for deciding interpolation vs. extrapolation;
    std::vector<Method> methods;
    std::vector<Method> previous_methods;
    std::vector<std::vector<short>> hypercube; // A minimal set of indices near the target needed to
                                               // perform interpolation calculations.
    bool reset_hypercube {false};
    std::vector<std::vector<double>>
        weighting_factors;       // weights of hypercube neighbor grid point data used
                                 // to calculate the value at the target
    std::vector<double> results; // Interpolated results at a given target

    std::vector<std::vector<double>> interpolation_coefficients;
    std::vector<std::vector<double>> cubic_slope_coefficients;

    std::vector<std::vector<double>> hypercube_grid_point_data;
    std::vector<double> hypercube_weights;

    std::map<std::pair<std::size_t, std::size_t>, std::vector<std::vector<double>>> hypercube_cache;

    std::size_t hypercube_size_hash {0u};

    // Internal methods
    std::size_t get_grid_point_index_relative(const std::vector<std::size_t>& coordinates,
                                              const std::vector<short>& translation);

    void setup();

    void set_axes_parent_pointers();

    void check_grid_point_data_set_size(const GridPointDataSet& grid_point_data_set);

    void calculate_floor_to_ceiling_fractions();

    void consolidate_methods();

    void calculate_interpolation_coefficients();

    void set_hypercube(std::vector<Method> methods);

    void set_hypercube_grid_point_data();

    void set_results();

    void set_floor_grid_point_coordinates();

    void set_axis_floor_grid_point_index(std::size_t axis_index);

    void check_axis_index(std::size_t axis_index, const std::string& action_description) const
    {
        if (axis_index > number_of_grid_axes - 1) {
            send_error(fmt::format(
                "Axis index, {}, does not exist. Unable to {}. Number of grid axes = {}.",
                axis_index,
                action_description,
                number_of_grid_axes));
        }
    }

    void check_data_set_index(std::size_t data_set_index,
                              const std::string& action_description) const
    {
        if (data_set_index > number_of_grid_point_data_sets - 1) {
            send_error(fmt::format("Data set index, {}, does not exist. Unable to {}. Number of "
                                   "grid point data sets = {}.",
                                   data_set_index,
                                   action_description,
                                   number_of_grid_point_data_sets));
        }
    }
};

std::vector<GridAxis> construct_grid_axes(const std::vector<std::vector<double>>& grid,
                                          const std::shared_ptr<Courier::Courier>& courier_in);

std::vector<GridPointDataSet>
construct_grid_point_data_sets(const std::vector<std::vector<double>>& grid_point_data_sets);

inline double compute_fraction(const double x, const double start, const double end)
{
    // how far along an edge is the target?
    return (x - start) / (end - start);
}

} // namespace Btwxt
