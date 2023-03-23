/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

GridAxis::GridAxis(std::vector<double> grid_vector, std::shared_ptr<Courierr::Courierr> logger,
                   Method extrapolation_method, Method interpolation_method,
                   std::pair<double, double> extrapolation_limits)
    : grid(std::move(grid_vector)),
      spacing_multipliers(2, std::vector<double>(std::max((int)grid.size() - 1, 0), 1.0)),
      extrapolation_method(extrapolation_method),
      interpolation_method(interpolation_method),
      extrapolation_limits(std::move(extrapolation_limits)),
      gridaxis_logger(logger) {
  if (!logger) {
    throw std::exception(); // TODO: correct exception
  }
  if (grid.size() == 0) {
    throw BtwxtException("Cannot create a GridAxis from a zero-length vector.", *logger);
  }
  check_grid_sorted();
  check_extrap_limits();
  if (interpolation_method == Method::CUBIC) {
    calc_spacing_multipliers();
  }
}

void GridAxis::set_interpolation_method(Method interpolation_method) {
  this->interpolation_method = interpolation_method;
  if (interpolation_method == Method::CUBIC) {
    calc_spacing_multipliers();
  }
}

void GridAxis::calc_spacing_multipliers() {
  // "0" and "1" are the "flavors" of the calc_spacing_multipliers.
  // If you are sitting at the "0" along an edge of the hypercube, you want the "0" flavor
  if (grid.size() == 1) {
    interpolation_method = Method::LINEAR;
    gridaxis_logger->info(
        "A cubic interpolation method is not valid for grid axes with only one value. "
        "Interpolation method reset to linear.");
  }
  double center_spacing;
  for (std::size_t i = 0; i < grid.size() - 1; i++) {
    center_spacing = grid[i + 1] - grid[i];
    if (i != 0) {
      spacing_multipliers[0][i] = center_spacing / (grid[i + 1] - grid[i - 1]);
    }
    if (i + 2 != grid.size()) {
      spacing_multipliers[1][i] = center_spacing / (grid[i + 2] - grid[i]);
    }
  }
}

void GridAxis::check_grid_sorted() {
  bool grid_is_sorted = free_check_sorted(grid);
  if (!grid_is_sorted) {
    throw BtwxtException("Axis is not sorted.", *gridaxis_logger);
  }
}

void GridAxis::check_extrap_limits() {
  if (extrapolation_limits.first > grid[0]) {
    gridaxis_logger->info(fmt::format("The lower extrapolation limit ({}) is within the set of "
                                      "grid values. Setting to smallest grid value ({}).",
                                      extrapolation_limits.first, grid[0]));
    extrapolation_limits.first = grid[0];
  }
  if (extrapolation_limits.second < grid.back()) {
    gridaxis_logger->info(fmt::format("The upper extrapolation limit ({}) is within the set of "
                                      "grid values. Setting to largest grid value ({}).",
                                      extrapolation_limits.second, grid.back()));
    extrapolation_limits.second = grid.back();
  }
}

double GridAxis::get_spacing_multiplier(const std::size_t &flavor, const std::size_t &index) const {
  return spacing_multipliers[flavor][index];
}

} // namespace Btwxt
