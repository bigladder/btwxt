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
#include <btwxt/grid-axis.h>
#include <btwxt/regular-grid-interpolator.h>

namespace Btwxt {

enum class Bounds { OUTLAW, OUTBOUNDS, INBOUNDS };

class RegularGridInterpolatorPrivate {
public:
  RegularGridInterpolatorPrivate() = default;

  RegularGridInterpolatorPrivate(const std::vector<std::vector<double>> &grid,
                                 std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolatorPrivate(const std::vector<GridAxis> &grid,
                                 std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolatorPrivate(const std::vector<std::vector<double>> &grid,
                                 const std::vector<std::vector<double>> &values,
                                 std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolatorPrivate(const std::vector<GridAxis> &grid,
                                 const std::vector<std::vector<double>> &values,
                                 std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolatorPrivate(const RegularGridInterpolatorPrivate &source);

  RegularGridInterpolatorPrivate(const RegularGridInterpolatorPrivate &source,
                                 std::shared_ptr<Courierr::Courierr> messenger);

  // Add value table to GriddedData
  std::size_t add_value_table(const std::vector<double> &value_vector);

  // GridPoint gets instantiated inside calculate_value_at_target
  double get_value_at_target(const std::vector<double> &target, std::size_t table_index);

  double operator()(std::vector<double> target, std::size_t table_index) {
    return get_value_at_target(std::move(target), table_index);
  }

  double get_value_at_target(std::size_t table_index);

  double operator()(std::size_t table_index) { return get_value_at_target(table_index); }

  std::vector<double> get_values_at_target();

  std::vector<double> get_values_at_target(const std::vector<double> &target);

  std::vector<double> operator()(std::vector<double> target) {
    return get_values_at_target(std::move(target));
  }

  std::vector<double> operator()() { return get_values_at_target(); }

  void set_new_target(const std::vector<double> &target);

  void normalize_values_at_target(const double scalar = 1.0);

  void normalize_values_at_target(const std::vector<double> &target, const double scalar = 1.0);

  double normalize_values_at_target(std::size_t table_index, const double scalar = 1.0);

  double normalize_values_at_target(std::size_t table_index, const std::vector<double> &target,
                                    const double scalar = 1.0);

  std::vector<double> get_current_target();

  void clear_current_target();

  std::size_t get_ndims() const;

  std::size_t get_num_tables() const;

  void set_axis_interp_method(std::size_t dim, Method method) {
    grid_axes[dim].interpolation_method = method;
  }

  void set_axis_extrap_method(const std::size_t &dim, Method method) {
    grid_axes[dim].extrapolation_method = method;
  }

  void set_axis_extrap_limits(const std::size_t &dim,
                              const std::pair<double, double> &extrap_limits) {
    grid_axes[dim].set_extrap_limits(extrap_limits);
  }

  std::pair<double, double> get_extrap_limits(int dim);

  std::size_t get_value_index(const std::vector<std::size_t> &coords) const;

  std::size_t get_value_index_relative(const std::vector<std::size_t> &coords,
                                       const std::vector<short> &translation);

  void set_target(const std::vector<double> &v);

  std::vector<std::size_t> get_floor();

  std::vector<double> get_weights();

  std::vector<Bounds> get_is_inbounds();

  std::vector<Method> get_methods();

  std::vector<std::vector<double>> get_interp_coeffs();

  std::vector<std::vector<double>> get_cubic_slope_coeffs();

  std::vector<double> get_results();

  std::pair<double, double> get_extrap_limits(const std::size_t dim) const;

  double get_vertex_weight(const std::vector<short> &v);

  void normalize_grid_values_at_target(const double scalar = 1.0);

  double normalize_grid_values_at_target(std::size_t table_num, const double scalar = 1.0);

  void normalize_value_table(std::size_t table_num, double scalar = 1.0);

  void set_floor();

  double get_axis_spacing_mult(const std::size_t dim, const std::size_t flavor,
                               const std::size_t index) const;

  std::string write_data();

  void set_logger(std::shared_ptr<Courierr::Courierr> logger, bool set_gridaxes = false);

  Courierr::Courierr &get_logger() { return *btwxt_logger; };

  // TODO private:
  friend class TwoDFixtureWithLoggingContext;
  friend class TwoDFixtureWithLoggingContext_set_message_context_Test;

  static std::vector<GridAxis> construct_axes(const std::vector<std::vector<double>> &grid, std::shared_ptr<Courierr::Courierr> logger) {
    std::vector<GridAxis> grid_axes;
    for (const auto &axis : grid) {
      grid_axes.emplace_back(axis, logger);
    }
    return grid_axes;
  }

  std::vector<double> get_values(const std::vector<std::size_t> &coords);

  std::vector<double> get_values_relative(const std::vector<std::size_t> &coords,
                                          const std::vector<short> &translation);

  std::vector<double> get_values(const std::size_t index);
  const std::vector<double> &get_grid_vector(const std::size_t dim);

  std::vector<Method> get_interp_methods() const;

  std::vector<Method> get_extrap_methods() const;

  void calculate_weights();

  void consolidate_methods();

  void calculate_interp_coeffs();

  void set_dim_floor(std::size_t dim);

  void set_hypercube();

  void set_hypercube(std::vector<Method> methods);

  std::vector<std::vector<short>> &get_hypercube();

  void set_hypercube_values();

  void set_results();

  // Structured data
  std::vector<GridAxis> grid_axes;
  std::vector<std::vector<double>> value_tables;
  std::size_t num_values{0u};
  std::size_t num_tables{0u};
  std::size_t ndims{0u};
  std::vector<std::size_t> dimension_lengths;
  std::vector<std::size_t> dimension_step_size;
  std::vector<std::size_t> temp_coords;
  std::vector<double> value_set; // Pre-sized container to hold set of values at any coordinates

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
      weighting_factors; // A set of weighting factors for each dimension
  std::vector<double> results;

  std::vector<std::vector<double>> interp_coeffs;
  std::vector<std::vector<double>> cubic_slope_coeffs;

  std::vector<std::vector<double>> hypercube_values;
  std::vector<double> hypercube_weights;

  std::map<std::pair<std::size_t, std::size_t>, std::vector<std::vector<double>>> hypercube_cache;

  std::size_t hypercube_size_hash{0u};

  std::shared_ptr<Courierr::Courierr> btwxt_logger;
  void set_dimension_sizes();
};

inline double compute_fraction(double x, double edge[2]) {
  // how far along an edge is the target?
  return (x - edge[0]) / (edge[1] - edge[0]);
}

} // namespace Btwxt
