/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// btwxt
#include <btwxt/btwxt.h>

namespace Btwxt {

GridAxis::GridAxis(std::vector<double> values_in,
                   const std::string& name,
                   Method interpolation_method,
                   Method extrapolation_method,
                   std::pair<double, double> extrapolation_limits,
                   const std::shared_ptr<Courierr::Courierr>& logger_in)
    : name(name)
    , values(std::move(values_in))
    , extrapolation_method(extrapolation_method)
    , interpolation_method(interpolation_method)
    , extrapolation_limits(std::move(extrapolation_limits))
    , cubic_spacing_ratios_floor(
          4, std::vector<double>(std::max(static_cast<int>(values.size()), 0), 0.0))
    , cubic_spacing_ratios_ceiling(
          4, std::vector<double>(std::max(static_cast<int>(values.size()), 0), 0.0))
    , logger(logger_in)
{
    if (values.empty()) {
        throw BtwxtException(
            fmt::format("Cannot create grid axis (name=\"{}\") from a zero-length vector.", name),
            *logger);
    }
    check_grid_sorted();
    check_extrapolation_limits();
    if (interpolation_method == Method::cubic) {
        calculate_cubic_spacing_ratios();
    }
}

void GridAxis::set_interpolation_method(Method interpolation_method_in)
{
    interpolation_method = interpolation_method_in;
    if (interpolation_method_in == Method::cubic) {
        calculate_cubic_spacing_ratios();
    }
}

void GridAxis::set_extrapolation_method(Method extrapolation_method_in)
{
    constexpr std::string_view info_format =
        "A {} extrapolation method is not valid for grid axis (name=\"{}\") with only {} value. "
        "Extrapolation method reset to {}.";
    switch (extrapolation_method_in) {
    case Method::linear: {
        if (get_length() == 1) {
            extrapolation_method = Method::constant;
            logger->info(fmt::format(info_format, "linear", name, "one", "constant"));
            return;
        }
        break;
    }
    case Method::cubic: {
        if (get_length() <= 1) {
            extrapolation_method = Method::constant;
            logger->info(fmt::format(info_format, "cubic", name, "one", "constant"));
            return;
        }
        else if (get_length() == 2) {
            extrapolation_method = Method::linear;
            logger->info(fmt::format(info_format, "cubic", name, "two", "linear"));
            return;
        }
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
        interpolation_method = Method::linear;
        logger->info(fmt::format(
            "A cubic interpolation method is not valid for grid axis (name=\"{}\") with "
            "only one value. Interpolation method reset to linear.",
            name));
    }
    if (interpolation_method == Method::linear) {
        return;
    }
    for (std::size_t i = 0; i < values.size(); i++)
    {
        if ((i > 0) && (i + 1 < values.size()))
        {
    		    double w_m1 = values[i] - values[i - 1];
    		    double w_0 = values[i + 1] - values[i];
				    cubic_spacing_ratios_floor[0][i] = -w_0 * w_0 / w_m1 / (w_0 + w_m1);
				    cubic_spacing_ratios_floor[1][i] = (w_0 - w_m1) / w_m1;
				    cubic_spacing_ratios_floor[2][i] = w_m1 / (w_0 + w_m1);
        }
        if (i + 2 < values.size())
        {
    		    double w_0 = values[i + 1] - values[i];
    		    double w_1 = values[i + 2] - values[i + 1];
				    cubic_spacing_ratios_ceiling[1][i] = -w_1 / (w_0 + w_1);
				    cubic_spacing_ratios_ceiling[2][i] = (w_1 - w_0) / w_1;
				    cubic_spacing_ratios_ceiling[3][i] = w_0 * w_0 / w_1 / (w_0 + w_1);
        }
    }
}

const std::vector<double>&
GridAxis::get_cubic_spacing_ratios_floor(const std::size_t index) const
{
    return cubic_spacing_ratios_floor[index];
}

const std::vector<double>&
GridAxis::get_cubic_spacing_ratios_ceiling(const std::size_t index) const
{
    return cubic_spacing_ratios_ceiling[index];
}

void GridAxis::check_grid_sorted()
{
    bool grid_is_sorted = vector_is_sorted(values);
    if (!grid_is_sorted) {
        throw BtwxtException(fmt::format("Grid axis (name=\"{}\") values are not sorted.", name),
                             *logger);
    }
}

void GridAxis::check_extrapolation_limits()
{
    constexpr std::string_view info_format {
        "Grid axis (name=\"{}\") {} extrapolation limit ({}) is within the set of grid axis "
        "values. Setting to {} axis value ({})."};
    if (extrapolation_limits.first > values[0]) {
        logger->info(fmt::format(
            info_format, name, "lower", extrapolation_limits.first, "smallest", values[0]));
        extrapolation_limits.first = values[0];
    }
    if (extrapolation_limits.second < values.back()) {
        logger->info(fmt::format(
            info_format, name, "upper", extrapolation_limits.second, "largest", values.back()));
        extrapolation_limits.second = values.back();
    }
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
