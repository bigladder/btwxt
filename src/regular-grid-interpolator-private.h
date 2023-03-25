/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

// Standard
#include <functional>
#include <map>
#include <memory>
#include <vector>

// vendor
#include <courierr/courierr.h>

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

enum class Bounds { outlaw, out_of_bounds, in_bounds };

class RegularGridInterpolatorPrivate {
public:
  RegularGridInterpolatorPrivate() = default;

  RegularGridInterpolatorPrivate(const std::vector<std::vector<double>> &grid,
                                 std::shared_ptr<Courierr::Courierr> logger);

  RegularGridInterpolatorPrivate(const std::vector<GridAxis> &grid,
                                 std::shared_ptr<Courierr::Courierr> logger);

  RegularGridInterpolatorPrivate(const std::vector<std::vector<double>> &grid,
                                 const std::vector<std::vector<double>> &values,
                                 std::shared_ptr<Courierr::Courierr> logger);

  RegularGridInterpolatorPrivate(const std::vector<GridAxis> &grid,
                                 const std::vector<std::vector<double>> &values,
                                 std::shared_ptr<Courierr::Courierr> logger);

  // Data manipulation and settings
  std::size_t add_value_table(const std::vector<double> &value_vector);

  void set_axis_interpolation_method(std::size_t axis, Method method) {
    if (axis > number_of_axes - 1) {
      throw BtwxtException(fmt::format("Unable to set axis interpolation method on axis {}. "
                                       "Number of grid axes = {}.",
                                       axis, number_of_axes),
                           *logger);
    }
    grid_axes[axis].set_interpolation_method(method);
  }

  void set_axis_extrapolation_method(const std::size_t axis, Method method) {
    if (axis > number_of_axes - 1) {
      throw BtwxtException(fmt::format("Unable to set axis extrapolation method on axis {}. "
                                       "Number of grid axes = {}.",
                                       axis, number_of_axes),
                           *logger);
    }
    grid_axes[axis].set_extrapolation_method(method);
  }

  void set_axis_extrapolation_limits(const std::size_t axis,
                                     const std::pair<double, double> &limits) {
    if (axis > number_of_axes - 1) {
      throw BtwxtException(fmt::format("Unable to set axis extrapolation limits on axis {}. "
                                       "Number of grid axes = {}.",
                                       axis, number_of_axes),
                           *logger);
    }
    grid_axes[axis].set_extrapolation_limits(limits);
  }

  // Public methods (mirrored)
  void set_target(const std::vector<double> &target);

  [[nodiscard]] std::vector<double> get_target() const;

  void clear_target();

  std::vector<double> get_results();

  std::vector<double> get_results(const std::vector<double> &target);

  void normalize_grid_values_at_target(double scalar = 1.0);

  double normalize_grid_values_at_target(std::size_t table_num, double scalar = 1.0);

  void normalize_value_table(std::size_t table_num, double scalar = 1.0);

  std::string write_data();

  void set_logger(std::shared_ptr<Courierr::Courierr> logger, bool set_grid_axes_loggers = false);

  // Public getters
  [[nodiscard]] std::pair<double, double> get_extrapolation_limits(std::size_t axis) const;

  // Internal methods
  [[nodiscard]] std::size_t get_grid_point_index(const std::vector<std::size_t> &coords) const;

  std::size_t get_grid_point_index_relative(const std::vector<std::size_t> &coordinates,
                                            const std::vector<short> &translation);

  double get_grid_point_weighting_factor(const std::vector<short> &v);

  void set_floor_grid_point_coordinates();

  void set_axis_floor_grid_point_index(std::size_t axis);

  [[nodiscard]] const std::vector<double> &
  get_axis_cubic_spacing_ratios(std::size_t axis, std::size_t floor_or_ceiling) const;

  static std::vector<GridAxis> construct_axes(const std::vector<std::vector<double>> &grid,
                                              std::shared_ptr<Courierr::Courierr> logger_in) {
    std::vector<GridAxis> grid_axes;
    grid_axes.reserve(grid.size());
    for (const auto &axis : grid) {
      grid_axes.emplace_back(axis, logger_in);
    }
    return grid_axes;
  }

  const std::vector<double> &get_grid_point_values(const std::vector<std::size_t> &coords);
  const std::vector<double> &get_grid_point_values(const size_t index);

  std::vector<double> get_grid_point_values_relative(const std::vector<std::size_t> &coords,
                                                     const std::vector<short> &translation);

  const std::vector<double> &get_axis_values(const size_t axis);

  [[nodiscard]] std::vector<Method> get_interpolation_methods() const;

  [[nodiscard]] std::vector<Method> get_extrapolation_methods() const;

  void calculate_floor_to_ceiling_fractions();

  void consolidate_methods();

  void calculate_interpolation_coefficients();

  void set_hypercube(std::vector<Method> methods);

  std::vector<std::vector<short>> &get_hypercube();

  void set_hypercube_values();

  void set_results();

  // Structured data
  std::vector<GridAxis> grid_axes;
  std::vector<std::vector<double>> value_tables;
  std::size_t number_of_values{0u};
  std::size_t number_of_tables{0u};
  std::size_t number_of_axes{0u};
  std::vector<std::size_t>
      axis_lengths; // Number of points in each grid axis (size = number_of_axes)
  std::vector<std::size_t> axis_step_size;        // Used to translate grid point coordinates to
                                                  // indices (size = number_of_axes)
  std::vector<std::size_t> temporary_coordinates; // Memory placeholder to avoid re-allocating
                                                  // memory (size = number_of_axes)
  std::vector<double>
      grid_point_values; // Pre-sized container to store set of values at grid point coordinates

  // calculated data
  bool target_is_set{false};
  std::vector<double> target;
  std::vector<std::size_t> floor_grid_point_coordinates; // coordinates of the grid point <= target
  std::size_t floor_grid_point_index{
      0u}; // Index of the floor_grid_point_coordinates (used for hypercube caching)
  std::vector<double> floor_to_ceiling_fractions; // for each axis, the fraction the target value is
                                                  // between its floor and ceiling axis values
  std::vector<Bounds> is_inbounds;                // for deciding interpolation vs. extrapolation;
  std::vector<Method> methods;
  std::vector<Method> previous_methods;
  std::vector<std::vector<short>> hypercube; // A minimal set of indices near the target needed to
                                             // perform interpolation calculations.
  bool reset_hypercube{false};
  std::vector<std::vector<double>> weighting_factors; // weights of hypercube neighbor values used
                                                      // to calculate the value at the target
  std::vector<double> results;                        // Interpolated results at a given target

  std::vector<std::vector<double>> interpolation_coefficients;
  std::vector<std::vector<double>> cubic_slope_coefficients;

  std::vector<std::vector<double>> hypercube_values;
  std::vector<double> hypercube_weights;

  std::map<std::pair<std::size_t, std::size_t>, std::vector<std::vector<double>>> hypercube_cache;

  std::size_t hypercube_size_hash{0u};

  std::shared_ptr<Courierr::Courierr> logger;
  void set_axis_sizes();
};

inline double compute_fraction(const double x, const double start, const double end) {
  // how far along an edge is the target?
  return (x - start) / (end - start);
}

} // namespace Btwxt
