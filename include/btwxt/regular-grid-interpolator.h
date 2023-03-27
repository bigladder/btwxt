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
#include "grid-point-data.h"
#include "logging.h"

namespace Btwxt {

class RegularGridInterpolatorPrivate;

enum class TargetBoundsStatus {
    below_lower_extrapolation_limit,
    extrapolate_low,
    interpolate,
    extrapolate_high,
    above_upper_extrapolation_limit
};

// this will be the public-facing class.
class RegularGridInterpolator {
  public:
    RegularGridInterpolator() = default;

    explicit RegularGridInterpolator(const std::vector<std::vector<double>>& grid,
                                     const std::shared_ptr<Courierr::Courierr>& logger =
                                         std::make_shared<BtwxtContextCourierr>());

    RegularGridInterpolator(const std::vector<std::vector<double>>& grid,
                            const std::vector<std::vector<double>>& data_sets,
                            const std::shared_ptr<Courierr::Courierr>& logger =
                                std::make_shared<BtwxtContextCourierr>());

    explicit RegularGridInterpolator(const std::vector<GridAxis>& grid,
                                     const std::shared_ptr<Courierr::Courierr>& logger =
                                         std::make_shared<BtwxtContextCourierr>());

    RegularGridInterpolator(const std::vector<GridAxis>& grid,
                            const std::vector<std::vector<double>>& data_sets,
                            const std::shared_ptr<Courierr::Courierr>& logger =
                                std::make_shared<BtwxtContextCourierr>());

    RegularGridInterpolator(const RegularGridInterpolator& source);

    RegularGridInterpolator(const RegularGridInterpolator& source,
                            const std::shared_ptr<Courierr::Courierr>& logger);

    RegularGridInterpolator(const std::vector<std::vector<double>>& grid,
                            const std::vector<GridPointData>& data_sets,
                            const std::shared_ptr<Courierr::Courierr>& logger =
                                std::make_shared<BtwxtContextCourierr>());

    RegularGridInterpolator(const std::vector<GridAxis>& grid,
                            const std::vector<GridPointData>& data_sets,
                            const std::shared_ptr<Courierr::Courierr>& logger =
                                std::make_shared<BtwxtContextCourierr>());

    RegularGridInterpolator& operator=(const RegularGridInterpolator& source);

    std::size_t add_grid_point_data_set(const std::vector<double>& grid_point_data,
                                        const std::string& name = "");

    // Get results
    void set_target(const std::vector<double>& target);

    double get_value_at_target(const std::vector<double>& target, std::size_t data_set_index);

    double operator()(const std::vector<double>& target, const std::size_t data_set_index)
    {
        return get_value_at_target(target, data_set_index);
    }

    double get_value_at_target(std::size_t data_set_index);

    double operator()(const std::size_t data_set_index)
    {
        return get_value_at_target(data_set_index);
    }

    std::vector<double> get_values_at_target(const std::vector<double>& target);

    std::vector<double> operator()(const std::vector<double>& target)
    {
        return get_values_at_target(target);
    }

    std::vector<double> get_values_at_target();

    std::vector<double> operator()() { return get_values_at_target(); }

    void normalize_grid_point_data_at_target(double scalar = 1.0);

    void normalize_grid_point_data_at_target(const std::vector<double>& target,
                                             double scalar = 1.0);

    double normalize_grid_point_data_at_target(std::size_t data_set_index,
                                               const double scalar = 1.0);

    double normalize_grid_point_data_at_target(std::size_t data_set_index,
                                               const std::vector<double>& target,
                                               double scalar = 1.0);

    const std::vector<double>& get_target();

    [[nodiscard]] const std::vector<TargetBoundsStatus>& get_target_bounds_status() const;

    void clear_target();

    void set_axis_interpolation_method(std::size_t axis, Method method);

    void set_axis_extrapolation_method(std::size_t axis, Method method);

    void set_axis_extrapolation_limits(std::size_t axis,
                                       const std::pair<double, double>& extrapolation_limits);

    std::pair<double, double> get_axis_extrapolation_limits(std::size_t axis);

    std::string write_data();

    void set_logger(const std::shared_ptr<Courierr::Courierr>& logger,
                    bool set_grid_axes_loggers = false);

    std::shared_ptr<Courierr::Courierr> get_logger();

  private:
    std::unique_ptr<RegularGridInterpolatorPrivate> regular_grid_interpolator;
};

} // namespace Btwxt