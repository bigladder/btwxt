/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

// Standard
#include <algorithm>
#include <cfloat>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

// btwxt
#include "logging.h"

namespace Btwxt {

enum class Method { undefined, constant, linear, cubic };

// free functions
inline bool free_check_sorted(const std::vector<double> &v) {
  return std::is_sorted(std::begin(v), std::end(v),
                        [](const double a, const double b) { return a <= b; });
}

class GridAxis {
  // A single input dimension of the performance space
public:
  // Constructors
  GridAxis() = default;

  explicit GridAxis(
      std::vector<double> grid_vector,
      std::shared_ptr<Courierr::Courierr> logger = std::make_shared<BtwxtContextCourierr>(),
      Method extrapolation_method = Method::constant, Method interpolation_method = Method::linear,
      std::pair<double, double> extrapolation_limits = {-DBL_MAX, DBL_MAX});

  // Setters
  void set_interpolation_method(Method interpolation_method_in);
  void set_extrapolation_method(Method extrapolation_method_in) {
    extrapolation_method = extrapolation_method_in;
  }

  void set_extrapolation_limits(std::pair<double, double> limits) {
    extrapolation_limits = limits;
    check_extrapolation_limits();
  }

  void set_logger(std::shared_ptr<Courierr::Courierr> logger_in) { logger = std::move(logger_in); }
  std::shared_ptr<Courierr::Courierr> get_logger() { return logger; };

  // Getters
  [[nodiscard]] std::vector<double> &get_values() { return values; }
  [[nodiscard]] std::size_t get_length() const { return values.size(); }
  [[nodiscard]] Method get_extrapolation_method() const { return extrapolation_method; }
  [[nodiscard]] Method get_interpolation_method() const { return interpolation_method; }
  [[nodiscard]] std::pair<double, double> get_extrapolation_limits() const {
    return extrapolation_limits;
  }

  [[nodiscard]] double get_spacing_multiplier(const std::size_t &flavor,
                                              const std::size_t &index) const;
  // TODO Make private
  std::vector<std::vector<double>>
      spacing_multipliers; // Used for cubic interpolation. For each point, represents the ratio of

private:
  std::vector<double> values;
  Method extrapolation_method{Method::constant};
  Method interpolation_method{Method::linear};
  std::pair<double, double> extrapolation_limits{-DBL_MAX, DBL_MAX};
  std::shared_ptr<Courierr::Courierr> logger;
  void calculate_spacing_multipliers();
  void check_grid_sorted();
  void check_extrapolation_limits();
};

} // namespace Btwxt
