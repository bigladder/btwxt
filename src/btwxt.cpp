/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>
#include <numeric>

// btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>> &grid, std::shared_ptr<Courierr::Courierr> messenger)
    : grid_data(grid, messenger), grid_point(grid_data, messenger), btwxt_logger(messenger) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>> &grid,
                                                 const std::vector<std::vector<double>> &values, std::shared_ptr<Courierr::Courierr> messenger)
    : grid_data(grid, values, messenger)
    , grid_point(grid_data, messenger)
    , btwxt_logger(messenger)
{
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source)
    : grid_data(source.grid_data), grid_point(source.grid_point), btwxt_logger(source.btwxt_logger) {
  *this = source;
}

std::size_t RegularGridInterpolator::add_value_table(const std::vector<double> &value_vector) {
  return grid_data.add_value_table(value_vector);
}

double RegularGridInterpolator::get_value_at_target(const std::vector<double>& target,
                                                    std::size_t table_index) {
  set_new_target(target);
  auto results = grid_point.get_results();
  return results[table_index];
}

double RegularGridInterpolator::get_value_at_target(std::size_t table_index) {
  auto results = grid_point.get_results();
  return results[table_index];
}

std::vector<double>
RegularGridInterpolator::get_values_at_target(const std::vector<double> &target) {
  set_new_target(target);
  return grid_point.get_results();
}

std::vector<double> RegularGridInterpolator::get_values_at_target() {
  return grid_point.get_results();
}

double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const std::vector<double> &target,
                                                           const double scalar) {
  set_new_target(target);
  return normalize_values_at_target(table_index, scalar);
}

double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const double scalar) {
  return grid_point.normalize_grid_values_at_target(table_index, scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const std::vector<double> &target,
                                                         const double scalar) {
  set_new_target(target);
  normalize_values_at_target(scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const double scalar) {
  return grid_point.normalize_grid_values_at_target(scalar);
}

void RegularGridInterpolator::set_new_target(const std::vector<double> &target) {
  grid_point.set_target(target);
}

std::vector<double> RegularGridInterpolator::get_current_target() {
  return grid_point.get_current_target();
}

void RegularGridInterpolator::clear_current_target() {
  grid_point = GridPoint(grid_data, btwxt_logger);
}

std::size_t RegularGridInterpolator::get_ndims() const { return grid_data.get_ndims(); }

std::size_t RegularGridInterpolator::get_num_tables() const { return grid_data.get_num_tables(); }

std::pair<double, double> RegularGridInterpolator::get_axis_limits(int dim) {
  return grid_data.get_extrap_limits(dim);
}

void RegularGridInterpolator::set_logger(std::shared_ptr<Courierr::Courierr> logger, bool set_gridaxes)
{
  btwxt_logger = logger;
  grid_point.set_logger(logger);
  if (set_gridaxes) {
    grid_data.set_logger(logger);
  }
}

} // namespace Btwxt
