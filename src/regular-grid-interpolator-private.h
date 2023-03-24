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

  void set_axis_interpolation_method(std::size_t dimension, Method method) {
    if (dimension > number_of_dimensions - 1) {
      throw BtwxtException(fmt::format("Unable to set axis interpolation method on dimension {}. "
                                       "Number of grid axes = {}.",
                                       dimension, number_of_dimensions),
                           *logger);
    }
    grid_axes[dimension].set_interpolation_method(method);
  }

  void set_axis_extrapolation_method(const std::size_t dimension, Method method) {
    if (dimension > number_of_dimensions - 1) {
      throw BtwxtException(fmt::format("Unable to set axis extrapolation method on dimension {}. "
                                       "Number of grid axes = {}.",
                                       dimension, number_of_dimensions),
                           *logger);
    }
    grid_axes[dimension].set_extrapolation_method(method);
  }

  void set_axis_extrapolation_limits(const std::size_t dimension,
                                     const std::pair<double, double> &limits) {
    if (dimension > number_of_dimensions - 1) {
      throw BtwxtException(fmt::format("Unable to set axis extrapolation limits on dimension {}. "
                                       "Number of grid axes = {}.",
                                       dimension, number_of_dimensions),
                           *logger);
    }
    grid_axes[dimension].set_extrapolation_limits(limits);
  }

  // Public calculations
  void set_target(const std::vector<double> &target);

  [[nodiscard]] std::vector<double> get_target() const;

  void clear_target();

  [[nodiscard]] std::pair<double, double> get_extrapolation_limits(std::size_t dimension) const;

  [[nodiscard]] std::size_t get_value_index(const std::vector<std::size_t> &coords) const;

  std::size_t get_value_index_relative(const std::vector<std::size_t> &coords,
                                       const std::vector<short> &translation);

  std::vector<std::size_t> get_floor();

  std::vector<double> get_weights();

  std::vector<Bounds> get_is_inbounds();

  std::vector<Method> get_methods();

  std::vector<std::vector<double>> &get_interpolation_coefficients();

  std::vector<std::vector<double>> &get_cubic_slope_coefficients();

  std::vector<double> get_results();

  std::vector<double> get_results(const std::vector<double> &target);

  double get_vertex_weight(const std::vector<short> &v);

  void normalize_grid_values_at_target(double scalar = 1.0);

  double normalize_grid_values_at_target(std::size_t table_num, double scalar = 1.0);

  void normalize_value_table(std::size_t table_num, double scalar = 1.0);

  void set_floor();

  [[nodiscard]] const std::vector<double> &get_axis_cubic_spacing_ratios(std::size_t dimension,
                                                                         std::size_t flavor) const;
  std::string write_data();

  void set_logger(std::shared_ptr<Courierr::Courierr> logger, bool set_grid_axes_loggers = false);

  static std::vector<GridAxis> construct_axes(const std::vector<std::vector<double>> &grid,
                                              std::shared_ptr<Courierr::Courierr> logger_in) {
    std::vector<GridAxis> grid_axes;
    grid_axes.reserve(grid.size());
    for (const auto &axis : grid) {
      grid_axes.emplace_back(axis, logger_in);
    }
    return grid_axes;
  }

  std::vector<double> get_values(const std::vector<std::size_t> &coords);

  std::vector<double> get_values_relative(const std::vector<std::size_t> &coords,
                                          const std::vector<short> &translation);

  std::vector<double> get_values(std::size_t index);
  const std::vector<double> &get_axis_values(const size_t dimension);

  [[nodiscard]] std::vector<Method> get_interpolation_methods() const;

  [[nodiscard]] std::vector<Method> get_extrapolation_methods() const;

  void calculate_weights();

  void consolidate_methods();

  void calculate_interpolation_coefficients();

  void set_dimension_floor(std::size_t dimension);

  void set_hypercube(std::vector<Method> methods);

  std::vector<std::vector<short>> &get_hypercube();

  void set_hypercube_values();

  void set_results();

  // Structured data
  std::vector<GridAxis> grid_axes;
  std::vector<std::vector<double>> value_tables;
  std::size_t number_of_values{0u};
  std::size_t number_of_tables{0u};
  std::size_t number_of_dimensions{0u};
  std::vector<std::size_t>
      dimension_lengths; // Number of points in each grid dimension (size = number_of_dimensions)
  std::vector<std::size_t> dimension_step_size;   // Used to translate grid point coordinates to
                                                  // indices (size = number_of_dimensions)
  std::vector<std::size_t> temporary_coordinates; // Memory placeholder to avoid re-allocating
                                                  // memory (size = number_of_dimensions)
  std::vector<double>
      value_set; // Pre-sized container to store set of values at values point coordinates

  // calculated data
  bool target_is_set{false};
  std::vector<double> target;
  std::vector<std::size_t> point_floor; // index of grid point <= target
  std::size_t floor_index{0u};
  std::vector<double> weights;
  std::vector<Bounds> is_inbounds; // for deciding interpolation vs. extrapolation;
  std::vector<Method> methods;
  std::vector<Method> previous_methods;
  std::vector<std::vector<short>> hypercube;
  bool reset_hypercube{false};
  std::vector<std::vector<double>>
      weighting_factors;       // A set of weighting factors for each dimension
  std::vector<double> results; // Interpolated results at a given target

  std::vector<std::vector<double>> interpolation_coefficients;
  std::vector<std::vector<double>> cubic_slope_coefficients;

  std::vector<std::vector<double>> hypercube_values;
  std::vector<double> hypercube_weights;

  std::map<std::pair<std::size_t, std::size_t>, std::vector<std::vector<double>>> hypercube_cache;

  std::size_t hypercube_size_hash{0u};

  std::shared_ptr<Courierr::Courierr> logger;
  void set_dimension_sizes();
};

inline double compute_fraction(const double x, const double start, const double end) {
  // how far along an edge is the target?
  return (x - start) / (end - start);
}

} // namespace Btwxt
