/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

#include <cfloat>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace Courierr {
class Courierr;
}

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

  GridAxis(std::vector<double> grid_vector, std::shared_ptr<Courierr::Courierr> logger,
           Method extrapolation_method = Method::CONSTANT,
           Method interpolation_method = Method::LINEAR,
           std::pair<double, double> extrapolation_limits = {-DBL_MAX, DBL_MAX});

  std::vector<double> grid;
  std::vector<std::vector<double>> spacing_multipliers;
  Method extrapolation_method;
  Method interpolation_method;
  std::pair<double, double> extrapolation_limits;

  std::size_t get_length() { return grid.size(); }

  void set_interp_method(Method interpolation_method);
  void set_extrap_method(Method extrapolation_method) {
    this->extrapolation_method = extrapolation_method;
  }

  void set_extrap_limits(std::pair<double, double> extrap_limits) {
    extrapolation_limits = extrap_limits;
    check_extrap_limits();
  }

  double get_spacing_multiplier(const std::size_t &flavor, const std::size_t &index) const;

  void set_logger(std::shared_ptr<Courierr::Courierr> logger) { gridaxis_logger = logger; }
  Courierr::Courierr &get_logger() { return *gridaxis_logger; };

private:
  std::shared_ptr<Courierr::Courierr> gridaxis_logger;
  void calc_spacing_multipliers();
  void check_grid_sorted();
  void check_extrap_limits();
};

template <typename T>
std::vector<std::vector<T>> cart_product(const std::vector<std::vector<T>> &v) {
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
