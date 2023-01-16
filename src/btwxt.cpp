/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>
#include <numeric>

// btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {

RegularGridInterpolator::RegularGridInterpolator() = default;

RegularGridInterpolator::RegularGridInterpolator(GriddedData &grid_data_in)
    : grid_data(grid_data_in), grid_point(grid_data) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>> &grid,
                                                 const std::vector<std::vector<double>> &values)
    : grid_data(grid, values), grid_point(grid_data) {}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source) {
  *this = source;
}

double RegularGridInterpolator::get_value_at_target(std::vector<double> target,
                                                    std::size_t table_index) {
  set_new_target(target);
  auto [results, err_is_null] = grid_point.get_results();
  if (!err_is_null) {
    handle_error_message(MsgLevel::MSG_WARN, err_is_null.value());
  }
  return results[table_index];
}

double RegularGridInterpolator::get_value_at_target(std::size_t table_index) {
  auto [results, err_is_null] = grid_point.get_results();
  if (!err_is_null) {
    handle_error_message(MsgLevel::MSG_WARN, err_is_null.value());
  }
  return results[table_index];
}

std::vector<double>
RegularGridInterpolator::get_values_at_target(const std::vector<double> &target) {
  set_new_target(target);
  auto [results, err_is_null] = grid_point.get_results();
  if (!err_is_null) {
    handle_error_message(MsgLevel::MSG_WARN, err_is_null.value());
  }
  return results;
}

std::vector<double> RegularGridInterpolator::get_values_at_target() {
  auto [results, err_is_null] = grid_point.get_results();
  if (!err_is_null) {
    handle_error_message(MsgLevel::MSG_WARN, err_is_null.value());
  }
  return results;
}

double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const std::vector<double> &target,
                                                           const double scalar) {
  set_new_target(target);
  return normalize_values_at_target(table_index, scalar);
}

double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const double scalar) {
  try {
    return grid_point.normalize_grid_values_at_target(table_index, scalar);
  } catch (BtwxtWarn &w) {
    handle_error_message(MsgLevel::MSG_WARN, w.what());
    return scalar; // TODO:
  }
}

void RegularGridInterpolator::normalize_values_at_target(const std::vector<double> &target,
                                                         const double scalar) {
  set_new_target(target);
  normalize_values_at_target(scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const double scalar) {
  try {
    return grid_point.normalize_grid_values_at_target(scalar);
  } catch (BtwxtWarn &w) {
    handle_error_message(MsgLevel::MSG_WARN, w.what());
  }
}

void RegularGridInterpolator::set_new_target(const std::vector<double> &target) {
  try {
    grid_point.set_target(target);
  } catch (BtwxtErr &e) {
    handle_error_message(MsgLevel::MSG_ERR, e.what());
  }
}

std::vector<double> RegularGridInterpolator::get_current_target() {
  auto [target, err_is_null] = grid_point.get_current_target();
  if (!err_is_null) {
    handle_error_message(MsgLevel::MSG_WARN, err_is_null.value());
  }
  return target;
}

void RegularGridInterpolator::clear_current_target() { grid_point = GridPoint(grid_data); }

std::size_t RegularGridInterpolator::get_ndims() { return grid_data.get_ndims(); }

std::vector<std::vector<short>> &RegularGridInterpolator::get_hypercube() {
  return grid_point.get_hypercube();
}

std::pair<double, double> RegularGridInterpolator::get_axis_limits(int dim) {
  return grid_data.get_extrap_limits(dim);
}

void RegularGridInterpolator::set_error_callback(BtwxtErrorHandler callback_function,
                                                 void *caller_info) {
  error_handler_ = std::move(callback_function);
  caller_context_ = caller_info;
}

void RegularGridInterpolator::handle_error_message(MsgLevel messageType, std::string_view message) {
  if (error_handler_) {
    error_handler_(messageType, message, caller_context_);
  } else if (btwxtCallbackFunction) {
    showMessage(messageType, std::string{message});
  }
}

} // namespace Btwxt
