/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

GridAxis::GridAxis(std::vector<double> values_in,
                   const std::shared_ptr<Courierr::Courierr> &logger_in,
                   Method extrapolation_method, Method interpolation_method,
                   std::pair<double, double> extrapolation_limits)
    : values(std::move(values_in)),
      extrapolation_method(extrapolation_method),
      interpolation_method(interpolation_method),
      extrapolation_limits(std::move(extrapolation_limits)),
      cubic_spacing_ratios(
          2, std::vector<double>(std::max(static_cast<int>(values.size()) - 1, 0), 1.0)),
      logger(logger_in) {
  if (values.empty()) {
    throw BtwxtException("Cannot create a GridAxis from a zero-length vector.", *logger);
  }
  check_grid_sorted();
  check_extrapolation_limits();
  if (interpolation_method == Method::cubic) {
    calculate_cubic_spacing_ratios();
  }
}

void GridAxis::set_interpolation_method(Method interpolation_method_in) {
  interpolation_method = interpolation_method_in;
  if (interpolation_method_in == Method::cubic) {
    calculate_cubic_spacing_ratios();
  }
}

void GridAxis::set_extrapolation_method(Method extrapolation_method_in) {
  switch (extrapolation_method_in) {
  case Method::linear: {
    if (get_length() == 1) {
      extrapolation_method = Method::constant;
      logger->info("A linear extrapolation method is not valid for grid axes with only one value. "
                   "Extrapolation method reset to constant.");
      return;
    }
    break;
  }
  case Method::cubic: {
    if (get_length() <= 1) {
      extrapolation_method = Method::constant;
      logger->info("A cubic extrapolation method is not valid for grid axes with only one value. "
                   "Extrapolation method reset to constant.");
      return;
    } else if (get_length() == 2) {
      extrapolation_method = Method::linear;
      logger->info("A cubic extrapolation method is not valid for grid axes with only two values. "
                   "Extrapolation method reset to linear.");
      return;
    }
  }
  default: {
    break;
  }
  }
  extrapolation_method = extrapolation_method_in;
}

void GridAxis::calculate_cubic_spacing_ratios() {
  if (get_length() == 1) {
    interpolation_method = Method::linear;
    logger->info("A cubic interpolation method is not valid for grid axes with only one value. "
                 "Interpolation method reset to linear.");
  }
  if (interpolation_method == Method::linear) {
    return;
  }
  static constexpr std::size_t floor = 0;
  static constexpr std::size_t ceiling = 1;
  for (std::size_t i = 0; i < values.size() - 1; i++) {
    double center_spacing = values[i + 1] - values[i];
    if (i != 0) {
      cubic_spacing_ratios[floor][i] = center_spacing / (values[i + 1] - values[i - 1]);
    }
    if (i + 2 != values.size()) {
      cubic_spacing_ratios[ceiling][i] = center_spacing / (values[i + 2] - values[i]);
    }
  }
}

const std::vector<double> &
GridAxis::get_cubic_spacing_ratios(const std::size_t floor_or_ceiling) const {
  return cubic_spacing_ratios[floor_or_ceiling];
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
