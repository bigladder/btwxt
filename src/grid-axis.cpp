/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// btwxt
#include <btwxt/btwxt.h>
#include "regular-grid-interpolator-implementation.h"

namespace Btwxt {

GridAxis::GridAxis(std::vector<double> values_in,
                   std::string name,
                   const std::shared_ptr<Courier::Courier>& courier_in,
                   InterpolationMethod interpolation_method,
                   ExtrapolationMethod extrapolation_method,
                   std::pair<double, double> extrapolation_limits)
    : name(std::move(name))
    , values(std::move(values_in))
    , interpolation_method(interpolation_method)
    , extrapolation_method(extrapolation_method)
    , extrapolation_limits(std::move(extrapolation_limits))
    , cubic_spacing_ratios(
          2, std::vector<double>(std::max(static_cast<int>(values.size()) - 1, 0), 1.0))
    , courier(courier_in)
{
    if (values.empty()) {
        send_error("Cannot create grid axis from a zero-length vector.");
    }
    check_grid_sorted();
    check_extrapolation_limits();
    if (interpolation_method == InterpolationMethod::cubic) {
        calculate_cubic_spacing_ratios();
    }
}

void GridAxis::set_interpolation_method(InterpolationMethod interpolation_method_in)
{
    interpolation_method = interpolation_method_in;
    if (interpolation_method_in == InterpolationMethod::cubic) {
        calculate_cubic_spacing_ratios();
    }
}

void GridAxis::set_extrapolation_method(ExtrapolationMethod extrapolation_method_in)
{
    switch (extrapolation_method_in) {
    case ExtrapolationMethod::linear: {
        if (get_length() == 1) {
            extrapolation_method = ExtrapolationMethod::constant;
            send_warning(
                "A linear extrapolation method is not valid for grid axis with only one value. "
                "Extrapolation method reset to constant.");
            return;
        }
        break;
    }
    default: {
        break;
    }
    }
    extrapolation_method = extrapolation_method_in;
}

void GridAxis::calculate_cubic_spacing_ratios()
{
    if (get_length() == 1) {
        interpolation_method = InterpolationMethod::linear;
        send_warning("A cubic interpolation method is not valid for grid axis with only one value. "
                     "Interpolation method reset to linear.");
    }
    if (interpolation_method == InterpolationMethod::linear) {
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

const std::vector<double>&
GridAxis::get_cubic_spacing_ratios(const std::size_t floor_or_ceiling) const
{
    return cubic_spacing_ratios[floor_or_ceiling];
}

void GridAxis::check_grid_sorted()
{
    bool grid_is_sorted = vector_is_valid(values);
    if (!grid_is_sorted) {
        send_error("Values are not sorted, or have duplicates.");
    }
}

void GridAxis::check_extrapolation_limits()
{
    constexpr std::string_view error_format {"{} extrapolation limit ({:.6g}) is within the range "
                                             "of grid axis values [{:.6g}, {:.6g}]."};
    if (extrapolation_limits.first > values[0]) {
        send_error(fmt::format(
            error_format, "Lower", extrapolation_limits.first, values[0], values.back()));
        extrapolation_limits.first = values[0];
    }
    if (extrapolation_limits.second < values.back()) {
        send_error(fmt::format(
            error_format, "Upper", extrapolation_limits.second, values[0], values.back()));
        extrapolation_limits.second = values.back();
    }
}

std::string GridAxis::make_message(const std::string& message) const
{
    std::string grid_axis_message = fmt::format("GridAxis '{}': {}", name, message);
    if (parent_interpolator) {
        return parent_interpolator->make_message(grid_axis_message);
    }
    return grid_axis_message;
}

// return an evenly spaced 1-d vector of doubles.
std::vector<double> linspace(double start, double stop, std::size_t number_of_points)
{
    std::vector<double> result(number_of_points);
    double step = (stop - start) / (static_cast<double>(number_of_points) - 1.);
    double value = start;
    for (std::size_t i = 0; i < number_of_points; i++) {
        result[i] = value;
        value += step;
    }
    return result;
}

} // namespace Btwxt
