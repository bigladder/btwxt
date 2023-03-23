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

enum class Method { CONSTANT, LINEAR, CUBIC, UNDEF };

// free functions
inline bool free_check_sorted(const std::vector<double> &v) {
  return std::is_sorted(std::begin(v), std::end(v),
                        [](const double a, const double b) { return a <= b; });
}

class GridAxis {
  // A single input dimension of the performance space
public:
  GridAxis() = default;

  GridAxis(std::vector<double> grid_vector,
           std::shared_ptr<Courierr::Courierr> logger = std::make_shared<BtwxtContextCourierr>(),
           Method extrapolation_method = Method::CONSTANT,
           Method interpolation_method = Method::LINEAR,
           std::pair<double, double> extrapolation_limits = {-DBL_MAX, DBL_MAX});

  std::vector<double> grid;
  std::vector<std::vector<double>> spacing_multipliers;
  Method extrapolation_method;
  Method interpolation_method;
  std::pair<double, double> extrapolation_limits;

  std::size_t get_length() { return grid.size(); }

  void set_interpolation_method(Method interpolation_method);
  void set_extrapolation_method(Method extrapolation_method) {
    this->extrapolation_method = extrapolation_method;
  }

  void set_extrapolation_limits(std::pair<double, double> limits) {
    extrapolation_limits = limits;
    check_extrapolation_limits();
  }

  double get_spacing_multiplier(const std::size_t &flavor, const std::size_t &index) const;

  void set_logger(std::shared_ptr<Courierr::Courierr> logger) { this->logger = logger; }
  std::shared_ptr<Courierr::Courierr> get_logger() { return logger; };

private:
  std::shared_ptr<Courierr::Courierr> logger;
  void calculate_spacing_multipliers();
  void check_grid_sorted();
  void check_extrapolation_limits();
};

template <typename T>
std::vector<std::vector<T>> cartesian_product(const std::vector<std::vector<T>> &v) {
  std::vector<std::vector<T>> combinations = {{}};
  for (const auto &list : v) {
    std::vector<std::vector<T>> r;
    for (const auto &x : combinations) {
      for (const auto item : list) {
        r.push_back(x);
        r.back().push_back(item);
      }
    }
    combinations = std::move(r);
  }
  return combinations;
}

} // namespace Btwxt
