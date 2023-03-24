/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

GridAxis::GridAxis(std::vector<double> values_in, std::shared_ptr<Courierr::Courierr> logger_in,
                   Method extrapolation_method, Method interpolation_method,
                   std::pair<double, double> extrapolation_limits)
    : values(std::move(values_in)),
      spacing_multipliers(
          2, std::vector<double>(std::max(static_cast<int>(values_in.size()) - 1, 0), 1.0)),
      extrapolation_method(extrapolation_method),
      interpolation_method(interpolation_method),
      extrapolation_limits(std::move(extrapolation_limits)),
      logger(logger_in) {
  if (!logger) {
    throw std::exception(); // TODO: correct exception
  }
  if (values.empty()) {
    throw BtwxtException("Cannot create a GridAxis from a zero-length vector.", *logger);
  }
  check_grid_sorted();
  check_extrapolation_limits();
  if (interpolation_method == Method::cubic) {
    calculate_spacing_multipliers();
  }
}

void GridAxis::set_interpolation_method(Method interpolation_method_in) {
  interpolation_method = interpolation_method_in;
  if (interpolation_method_in == Method::cubic) {
    calculate_spacing_multipliers();
  }
}

void GridAxis::calculate_spacing_multipliers() {
  // "0" and "1" are the "flavors" of the calculate_spacing_multipliers.
  // If you are sitting at the "0" along an edge of the hypercube, you want the "0" flavor
  if (values.size() == 1) {
    interpolation_method = Method::linear;
    logger->info("A cubic interpolation method is not valid for grid axes with only one value. "
                 "Interpolation method reset to linear.");
  }
  for (std::size_t i = 0; i < values.size() - 1; i++) {
    double center_spacing = values[i + 1] - values[i];
    if (i != 0) {
      spacing_multipliers[0][i] = center_spacing / (values[i + 1] - values[i - 1]);
    }
    if (i + 2 != values.size()) {
      spacing_multipliers[1][i] = center_spacing / (values[i + 2] - values[i]);
    }
  }
}

double GridAxis::get_spacing_multiplier(const std::size_t &flavor, const std::size_t &index) const {
  return spacing_multipliers[flavor][index];
}

void GridAxis::check_grid_sorted() {
  bool grid_is_sorted = free_check_sorted(values);
  if (!grid_is_sorted) {
    throw BtwxtException("Axis is not sorted.", *logger);
  }
}

void GridAxis::check_extrapolation_limits() {
  if (extrapolation_limits.first > values[0]) {
    logger->info(fmt::format("The lower extrapolation limit ({}) is within the set of "
                             "axis values. Setting to smallest axis value ({}).",
                             extrapolation_limits.first, values[0]));
    extrapolation_limits.first = values[0];
  }
  if (extrapolation_limits.second < values.back()) {
    logger->info(fmt::format("The upper extrapolation limit ({}) is within the set of "
                             "axis values. Setting to largest axis value ({}).",
                             extrapolation_limits.second, values.back()));
    extrapolation_limits.second = values.back();
  }
}

} // namespace Btwxt
