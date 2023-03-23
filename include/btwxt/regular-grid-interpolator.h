/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

// Standard
#include <functional>
#include <memory>
#include <vector>

#include <courierr/courierr.h>

// btwxt
#include "grid-axis.h"

namespace Btwxt {

class RegularGridInterpolatorPrivate;

// this will be the public-facing class.
class RegularGridInterpolator {
public:
  RegularGridInterpolator() = default;

  RegularGridInterpolator(const std::vector<std::vector<double>> &grid,
                          std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolator(const std::vector<std::vector<double>> &grid,
                          const std::vector<std::vector<double>> &values,
                          std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolator(const std::vector<GridAxis> &grid,
                          std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolator(const std::vector<GridAxis> &grid,
                          const std::vector<std::vector<double>> &values,
                          std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolator(const RegularGridInterpolator &source);

  RegularGridInterpolator(const RegularGridInterpolator &source,
                          std::shared_ptr<Courierr::Courierr> messenger);

  RegularGridInterpolator &operator=(const RegularGridInterpolator &source);

  // Add value table
  std::size_t add_value_table(const std::vector<double> &value_vector);

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

  void set_axis_interp_method(const std::size_t dim, const Method method);

  void set_axis_extrap_method(const std::size_t dim, const Method method);

  void set_axis_extrap_limits(const std::size_t dim,
                              const std::pair<double, double> &extrap_limits);

  std::pair<double, double> get_axis_limits(int dim);

  std::string write_data();

  void set_logger(std::shared_ptr<Courierr::Courierr> logger, bool set_gridaxes = false);

  Courierr::Courierr &get_logger();

private:
  std::unique_ptr<RegularGridInterpolatorPrivate> regular_grid_interpolator;
};

} // namespace Btwxt
