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
                   SlopeMethod slope_method_in,
                   double slope_reduction_in,
                   const std::shared_ptr<Courierr::Courierr>& logger_in)
    : name(name)
    , values(std::move(values_in))
    , extrapolation_method(extrapolation_method)
    , interpolation_method(interpolation_method)
    , slope_method(slope_method_in)
    , slope_reduction(slope_reduction_in)
    , extrapolation_limits(std::move(extrapolation_limits))
    , cubic_spacing_ratios(
        std::max(static_cast<int>(values.size()) - 1, 0),
        { { -1.0, 0.0 }, { 0.0, -1.0 }, { 1.0, 0.0 }, { 0.0, 1.0 } })
    , logger(logger_in) {
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

void GridAxis::set_slope_method(SlopeMethod slope_method_in)
{
    slope_method = slope_method_in;
}

void GridAxis::set_slope_reduction(double slope_reduction_in)
{
    slope_reduction = slope_reduction_in;
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
    for (std::size_t i_elem = 0; i_elem < values.size() - 1; i_elem++)
    {
        auto& ratio = cubic_spacing_ratios[i_elem];
        double w_0 = values[i_elem + 1] - values[i_elem];
        if (i_elem > 0)
        {
            double w_m1 = values[i_elem] - values[i_elem - 1];
            double t_0 = w_0 / w_m1;
            
            double c_0(0.0);
            switch (slope_method)
            {
                case SlopeMethod::finite_diff:{
                    c_0 = 0.5;
                    break;
                }
                case SlopeMethod::cardinal:{
                    c_0 = 1 / (1 + t_0);
                    break;
                }
                case SlopeMethod::quadratic:
                default:{
                    c_0 = t_0 / (1 + t_0);
                    break;
                }
            }
            
            //general
            double s_m1_m = (1 - slope_reduction) * (-t_0 * c_0);
            double s_1_m = (1 - slope_reduction) * (1 - c_0);

            ratio[0].first = s_m1_m;
            ratio[1].first = -(s_m1_m + s_1_m);
            ratio[2].first = s_1_m;
            ratio[3].first = 0.0;
        }
        if (i_elem + 2 < values.size())
        {
            double w_1 = values[i_elem + 2] - values[i_elem + 1];
            double t_1 = w_1 / w_0;

            double c_1(0.0);
            switch (slope_method){
                case SlopeMethod::finite_diff:
                {
                    c_1 = 0.5;
                    break;
                }
                case SlopeMethod::cardinal:{
                    c_1 = 1 / (1 + t_1);
                    break;
                }
                case SlopeMethod::quadratic:
                default:{
                    c_1 = t_1 / (1 + t_1);
                    break;
                }
            }

            double s_0_p = (1 - slope_reduction) * (-c_1);
            double s_2_p = (1 - slope_reduction) * ((1 - c_1) / t_1);

            ratio[0].second = 0.0;
            ratio[1].second = s_0_p;
            ratio[2].second = -(s_0_p + s_2_p);
            ratio[3].second = s_2_p;
        }
    }
}

const std::vector<std::pair<double,double>>&
GridAxis::get_cubic_spacing_ratios(const std::size_t elem_index) const
{
    return cubic_spacing_ratios[elem_index];
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
