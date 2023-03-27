/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>
#include <numeric>
#include <sstream> // std::stringstream

// btwxt
#include <btwxt/btwxt.h>

// btwxt private
#include "regular-grid-interpolator-private.h"

namespace Btwxt {

std::vector<GridAxis> construct_axes(const std::vector<std::vector<double>>& grid,
                                     const std::shared_ptr<Courierr::Courierr>& logger_in)
{
    std::vector<GridAxis> grid_axes;
    grid_axes.reserve(grid.size());
    for (const auto& axis : grid) {
        grid_axes.emplace_back(axis, logger_in);
    }
    return grid_axes;
}

std::vector<GridPointData>
construct_grid_point_data_sets(const std::vector<std::vector<double>>& grid_point_data_sets_in)
{
    std::vector<GridPointData> grid_point_data_sets;
    grid_point_data_sets.reserve(grid_point_data_sets_in.size());
    for (const auto& data_set : grid_point_data_sets_in) {
        grid_point_data_sets.emplace_back(data_set);
    }
    return grid_point_data_sets;
}

// Constructors
RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis>& grid,
                                                 const std::vector<GridPointData>& data_sets,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : regular_grid_interpolator(
          std::make_unique<RegularGridInterpolatorPrivate>(grid, data_sets, logger))
{
}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>>& grid,
                                                 const std::vector<std::vector<double>>& data_sets,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : RegularGridInterpolator(
          construct_axes(grid, logger), construct_grid_point_data_sets(data_sets), logger)
{
}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>>& grid,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : RegularGridInterpolator(construct_axes(grid, logger), std::vector<GridPointData>(), logger)
{
}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis>& grid,
                                                 const std::vector<std::vector<double>>& data_sets,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : regular_grid_interpolator(std::make_unique<RegularGridInterpolatorPrivate>(
          grid, construct_grid_point_data_sets(data_sets), logger))
{
}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>>& grid,
                                                 const std::vector<GridPointData>& data_sets,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : regular_grid_interpolator(std::make_unique<RegularGridInterpolatorPrivate>(
          construct_axes(grid, logger), data_sets, logger))
{
}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<GridAxis>& grid,
    const std::vector<GridPointData>& data_sets,
    const std::shared_ptr<Courierr::Courierr>& logger)
    : grid_axes(grid)
    , grid_point_data_sets(data_sets)
    , number_of_grid_point_data_sets(data_sets.size())
    , number_of_axes(grid.size())
    , axis_lengths(number_of_axes)
    , axis_step_size(number_of_axes)
    , temporary_coordinates(number_of_axes)
    , temporary_grid_point_data(number_of_grid_point_data_sets, 0.)
    , target(number_of_axes, 0.)
    , floor_grid_point_coordinates(number_of_axes, 0)
    , floor_to_ceiling_fractions(number_of_axes, 0.)
    , target_bounds_status(number_of_axes)
    , methods(number_of_axes, Method::undefined)
    , weighting_factors(number_of_axes, std::vector<double>(4, 0.))
    , results(number_of_grid_point_data_sets, 0.)
    , interpolation_coefficients(number_of_axes, std::vector<double>(2, 0.))
    , cubic_slope_coefficients(number_of_axes, std::vector<double>(2, 0.))
    , logger(logger)
{
    set_axis_sizes();
}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<GridAxis>& grid, const std::shared_ptr<Courierr::Courierr>& logger)
    : RegularGridInterpolatorPrivate(grid, {}, logger)
{
}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis>& grid,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : regular_grid_interpolator(std::make_unique<RegularGridInterpolatorPrivate>(grid, logger))
{
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator& source)
{
    *this = source;
    this->regular_grid_interpolator =
        std::make_unique<RegularGridInterpolatorPrivate>(*source.regular_grid_interpolator);
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator& source,
                                                 const std::shared_ptr<Courierr::Courierr>& logger)
    : RegularGridInterpolator(source)
{
    this->regular_grid_interpolator->logger = logger;
}

RegularGridInterpolator& RegularGridInterpolator::operator=(const RegularGridInterpolator& source)
{
    regular_grid_interpolator =
        std::make_unique<RegularGridInterpolatorPrivate>(*(source.regular_grid_interpolator));
    return *this;
}

void RegularGridInterpolatorPrivate::set_axis_sizes()
{
    number_of_grid_points = 1;
    for (std::size_t axis = number_of_axes - 1; /* axis >= 0 */ axis < number_of_axes; --axis) {
        std::size_t length = grid_axes[axis].get_length();
        if (length == 0) {
            throw BtwxtException(fmt::format("Axis {} has length zero.", axis), *logger);
        }
        axis_lengths[axis] = length;
        axis_step_size[axis] = number_of_grid_points;
        number_of_grid_points *= length;
    }
}

// Public manipulation methods

std::size_t
RegularGridInterpolator::add_grid_point_data_set(const std::vector<double>& grid_point_data,
                                                 const std::string& name)
{
    return regular_grid_interpolator->add_grid_point_data_set(grid_point_data);
}

std::size_t
RegularGridInterpolatorPrivate::add_grid_point_data_set(const std::vector<double>& grid_point_data)
{
    if (grid_point_data.size() != number_of_grid_points) {
        throw BtwxtException(
            fmt::format(
                "Input grid point data set size ({}) does not match number of grid points ({}).",
                grid_point_data.size(),
                number_of_grid_points),
            *logger);
    }
    grid_point_data_sets.emplace_back(grid_point_data);
    number_of_grid_point_data_sets++;
    temporary_grid_point_data.resize(number_of_grid_point_data_sets);
    results.resize(number_of_grid_point_data_sets);
    return number_of_grid_point_data_sets - 1; // Returns index of new data set
}

void RegularGridInterpolator::set_axis_extrapolation_method(const std::size_t axis,
                                                            const Method method)
{
    regular_grid_interpolator->set_axis_extrapolation_method(axis, method);
}

void RegularGridInterpolator::set_axis_interpolation_method(const std::size_t axis,
                                                            const Method method)
{
    regular_grid_interpolator->set_axis_interpolation_method(axis, method);
}

void RegularGridInterpolator::set_axis_extrapolation_limits(
    const std::size_t axis, const std::pair<double, double>& extrapolation_limits)
{
    regular_grid_interpolator->set_axis_extrapolation_limits(axis, extrapolation_limits);
}

// Public normalization methods
double RegularGridInterpolator::normalize_grid_point_data_at_target(
    std::size_t data_set_index, const std::vector<double>& target, const double scalar)
{
    set_target(target);
    return normalize_grid_point_data_at_target(data_set_index, scalar);
}

double RegularGridInterpolator::normalize_grid_point_data_at_target(std::size_t data_set_index,
                                                                    const double scalar)
{
    return regular_grid_interpolator->normalize_grid_point_data_at_target(data_set_index, scalar);
}

void RegularGridInterpolator::normalize_grid_point_data_at_target(const std::vector<double>& target,
                                                                  const double scalar)
{
    set_target(target);
    normalize_grid_point_data_at_target(scalar);
}

void RegularGridInterpolator::normalize_grid_point_data_at_target(const double scalar)
{
    return regular_grid_interpolator->normalize_grid_point_data_at_target(scalar);
}

void RegularGridInterpolatorPrivate::normalize_grid_point_data_at_target(const double scalar)
{
    if (!target_is_set) {
        throw BtwxtException(
            fmt::format("Cannot normalize grid point data. No target has been set."), *logger);
    }
    for (std::size_t data_set_index = 0; data_set_index < number_of_grid_point_data_sets;
         ++data_set_index) {
        normalize_grid_point_data(data_set_index, results[data_set_index] * scalar);
    }
    hypercube_cache.clear();
    set_results();
}

double
RegularGridInterpolatorPrivate::normalize_grid_point_data_at_target(std::size_t data_set_index,
                                                                    const double scalar)
{
    if (!target_is_set) {
        throw BtwxtException(
            fmt::format("Cannot normalize grid point data. No target has been set."), *logger);
    }
    // create a scalar which represents the product of the inverted normalization factor and the
    // value in the data set at the independent variable reference value
    double total_scalar = results[data_set_index] * scalar;
    normalize_grid_point_data(data_set_index, total_scalar);
    hypercube_cache.clear();
    set_results();

    return total_scalar;
}

void RegularGridInterpolatorPrivate::normalize_grid_point_data(std::size_t data_set_index,
                                                               double scalar)
{
    auto& data_set = grid_point_data_sets[data_set_index].data;
    if (scalar == 0.0) {
        throw BtwxtException("Attempt to normalize data_set by zero.", *logger);
    }
    scalar = 1.0 / scalar;
    std::transform(data_set.begin(),
                   data_set.end(),
                   data_set.begin(),
                   [scalar](double x) -> double { return x * scalar; });
}

// Public getter methods

std::pair<double, double>
RegularGridInterpolator::get_axis_extrapolation_limits(const std::size_t axis)
{
    return regular_grid_interpolator->get_extrapolation_limits(axis);
}

std::pair<double, double>
RegularGridInterpolatorPrivate::get_extrapolation_limits(std::size_t axis) const
{
    return grid_axes[axis].get_extrapolation_limits();
}

const GridAxis& RegularGridInterpolatorPrivate::get_grid_axis(const std::size_t axis) const
{
    return grid_axes[axis];
}

// Public printing methods
std::string RegularGridInterpolator::write_data()
{
    return regular_grid_interpolator->write_data();
}

std::string RegularGridInterpolatorPrivate::write_data()
{
    std::vector<std::size_t> indices(number_of_axes, 0);
    std::stringstream output("");

    for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
        output << "Axis " << axis + 1 << ",";
    }
    for (std::size_t tab = 0; tab < number_of_grid_point_data_sets; ++tab) {
        output << "Data " << tab + 1 << ",";
    }
    output << std::endl;
    for (std::size_t index = 0; index < number_of_grid_points; ++index) {
        for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
            output << get_grid_axis(axis).get_values()[indices[axis]] << ",";
        }
        for (std::size_t tab = 0; tab < number_of_grid_point_data_sets; ++tab) {
            output << grid_point_data_sets[tab].data[index] << ",";
        }
        output << std::endl;
        ++indices[number_of_axes - 1];
        for (std::size_t axis = number_of_axes - 1; axis > 0; --axis) {
            if (indices[axis] >= axis_lengths[axis]) {
                ++indices[axis - 1];
                indices[axis] = 0;
            }
        }
    }
    return output.str();
}

// Public calculation methods
void RegularGridInterpolator::set_target(const std::vector<double>& target)
{
    regular_grid_interpolator->set_target(target);
}

void RegularGridInterpolatorPrivate::set_target(const std::vector<double>& v)
{
    if (v.size() != number_of_axes) {
        throw BtwxtException(fmt::format("Target and grid do not have the same dimensions."),
                             *logger);
    }
    if (target_is_set) {
        if ((v == target) && (methods == get_interpolation_methods())) {
            return;
        }
    }
    target = v;
    target_is_set = true;
    set_floor_grid_point_coordinates();
    calculate_floor_to_ceiling_fractions();
    consolidate_methods();
    calculate_interpolation_coefficients();
    set_results();
}

void RegularGridInterpolatorPrivate::set_results()
{
    set_hypercube_grid_point_data();
    std::fill(results.begin(), results.end(), 0.0);
    for (std::size_t i = 0; i < hypercube.size(); ++i) {
        hypercube_weights[i] = get_grid_point_weighting_factor(hypercube[i]);
        for (std::size_t j = 0; j < number_of_grid_point_data_sets; ++j) {
            results[j] += hypercube_grid_point_data[i][j] * hypercube_weights[i];
        }
    }
}

double RegularGridInterpolator::get_value_at_target(const std::vector<double>& target,
                                                    std::size_t data_set_index)
{
    set_target(target);
    return get_value_at_target(data_set_index);
}

double RegularGridInterpolator::get_value_at_target(std::size_t data_set_index)
{
    return regular_grid_interpolator->get_results()[data_set_index];
}

std::vector<double> RegularGridInterpolatorPrivate::get_results() const
{
    if (number_of_grid_point_data_sets == 0u) {
        logger->warning(fmt::format("There are no grid point data sets. No results returned."));
    }
    if (!target_is_set) {
        logger->warning(fmt::format("Results were requested, but no target has been set."));
    }
    return results;
}

std::vector<double>
RegularGridInterpolatorPrivate::get_results(const std::vector<double>& target_in)
{
    set_target(target_in);
    return get_results();
}

std::vector<double> RegularGridInterpolator::get_values_at_target(const std::vector<double>& target)
{
    return regular_grid_interpolator->get_results(target);
}

std::vector<double> RegularGridInterpolator::get_values_at_target()
{
    return regular_grid_interpolator->get_results();
}

const std::vector<double>& RegularGridInterpolator::get_target()
{
    return regular_grid_interpolator->get_target();
}

const std::vector<TargetBoundsStatus>& RegularGridInterpolator::get_target_bounds_status() const
{
    return regular_grid_interpolator->target_bounds_status;
}

void RegularGridInterpolator::clear_target() { regular_grid_interpolator->clear_target(); }

// Public logging

void RegularGridInterpolator::set_logger(const std::shared_ptr<Courierr::Courierr>& logger,
                                         bool set_grid_axes_loggers)
{
    regular_grid_interpolator->set_logger(logger, set_grid_axes_loggers);
}

void RegularGridInterpolatorPrivate::set_logger(
    const std::shared_ptr<Courierr::Courierr>& logger_in, bool set_grid_axes_loggers)
{
    logger = logger_in;
    if (set_grid_axes_loggers) {
        for (auto& axis : grid_axes) {
            axis.set_logger(logger);
        }
    }
}

std::shared_ptr<Courierr::Courierr> RegularGridInterpolator::get_logger()
{
    return regular_grid_interpolator->logger;
}

// Internal calculation methods

std::size_t
RegularGridInterpolatorPrivate::get_grid_point_index(const std::vector<std::size_t>& coords) const
{
    std::size_t index = 0;
    for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
        index += coords[axis] * axis_step_size[axis];
    }
    return index;
}

const std::vector<double>& RegularGridInterpolatorPrivate::get_grid_point_data(size_t index)
{
    for (std::size_t i = 0; i < number_of_grid_point_data_sets; ++i) {
        temporary_grid_point_data[i] = grid_point_data_sets[i].data[index];
    }
    return temporary_grid_point_data;
}

const std::vector<double>&
RegularGridInterpolatorPrivate::get_grid_point_data(const std::vector<std::size_t>& coords)
{
    std::size_t index = get_grid_point_index(coords);
    return get_grid_point_data(index);
}

std::size_t RegularGridInterpolatorPrivate::get_grid_point_index_relative(
    const std::vector<std::size_t>& coords, const std::vector<short>& translation)
{
    int new_coord;
    for (std::size_t axis = 0; axis < coords.size(); axis++) {
        new_coord = static_cast<int>(coords[axis]) + translation[axis];
        if (new_coord < 0) {
            temporary_coordinates[axis] = 0u;
        }
        else if (new_coord >= static_cast<int>(axis_lengths[axis])) {
            temporary_coordinates[axis] = axis_lengths[axis] - 1u;
        }
        else {
            temporary_coordinates[axis] = new_coord;
        }
    }
    return get_grid_point_index(temporary_coordinates);
}

std::vector<double>
RegularGridInterpolatorPrivate::get_grid_point_data_relative(const std::vector<std::size_t>& coords,
                                                             const std::vector<short>& translation)
{
    return get_grid_point_data(get_grid_point_index_relative(coords, translation));
}

void RegularGridInterpolatorPrivate::set_floor_grid_point_coordinates()
{
    for (std::size_t axis = 0; axis < number_of_axes; axis += 1) {
        set_axis_floor_grid_point_index(axis);
    }
    floor_grid_point_index = get_grid_point_index(floor_grid_point_coordinates);
}

void RegularGridInterpolatorPrivate::set_axis_floor_grid_point_index(size_t axis)
{
    const auto& axis_values = get_grid_axis(axis).get_values();
    int length = static_cast<int>(axis_lengths[axis]);
    if (target[axis] < get_extrapolation_limits(axis).first) {
        target_bounds_status[axis] = TargetBoundsStatus::below_lower_extrapolation_limit;
        floor_grid_point_coordinates[axis] = 0u;
    }
    else if (target[axis] > get_extrapolation_limits(axis).second) {
        target_bounds_status[axis] = TargetBoundsStatus::above_upper_extrapolation_limit;
        floor_grid_point_coordinates[axis] =
            std::max(length - 2,
                     0); // length-2 because that's the left side of the (length-2, length-1) edge.
    }
    else if (target[axis] < axis_values[0]) {
        target_bounds_status[axis] = TargetBoundsStatus::extrapolate_low;
        floor_grid_point_coordinates[axis] = 0;
    }
    else if (target[axis] > axis_values.back()) {
        target_bounds_status[axis] = TargetBoundsStatus::extrapolate_high;
        floor_grid_point_coordinates[axis] =
            std::max(length - 2,
                     0); // length-2 because that's the left side of the (length-2, length-1) edge.
    }
    else if (target[axis] == axis_values.back()) {
        target_bounds_status[axis] = TargetBoundsStatus::interpolate;
        floor_grid_point_coordinates[axis] =
            std::max(length - 2,
                     0); // length-2 because that's the left side of the (length-2, length-1) edge.
    }
    else {
        target_bounds_status[axis] = TargetBoundsStatus::interpolate;
        auto upper = std::upper_bound(axis_values.begin(), axis_values.end(), target[axis]);
        floor_grid_point_coordinates[axis] = upper - axis_values.begin() - 1;
    }
}

void RegularGridInterpolatorPrivate::calculate_floor_to_ceiling_fractions()
{
    for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
        if (axis_lengths[axis] > 1) {
            auto& axis_values = get_grid_axis(axis).get_values();
            auto floor_index = floor_grid_point_coordinates[axis];
            floor_to_ceiling_fractions[axis] = compute_fraction(
                target[axis], axis_values[floor_index], axis_values[floor_index + 1]);
        }
        else {
            floor_to_ceiling_fractions[axis] = 1.0;
        }
    }
}

void RegularGridInterpolatorPrivate::consolidate_methods()
// If out of bounds, extrapolate according to prescription
// If outside of extrapolation limits, send a warning and perform constant extrapolation.
{
    previous_methods = methods;
    methods = get_interpolation_methods();
    if (target_is_set) {
        auto extrapolation_methods = get_extrapolation_methods();
        for (std::size_t axis = 0; axis < number_of_axes; axis++) {
            switch (target_bounds_status[axis]) {
            case TargetBoundsStatus::extrapolate_low:
            case TargetBoundsStatus::extrapolate_high:
                methods[axis] = extrapolation_methods[axis];
                break;
            case TargetBoundsStatus::below_lower_extrapolation_limit:
                logger->warning(fmt::format("The target ({:.3g}) is below the extrapolation limit "
                                            "({:.3g}) for axis {}. Result "
                                            "will use constant extrapolation from the lower limit.",
                                            target[axis],
                                            get_extrapolation_limits(axis).first,
                                            axis));
                methods[axis] = Method::constant;
                break;
            case TargetBoundsStatus::above_upper_extrapolation_limit:
                logger->warning(fmt::format("The target ({:.3g}) is above the extrapolation limit "
                                            "({:.3g}) for axis {}. Result "
                                            "will use constant extrapolation from the upper limit.",
                                            target[axis],
                                            get_extrapolation_limits(axis).second,
                                            axis));
                methods[axis] = Method::constant;
                break;
            case TargetBoundsStatus::interpolate:
                break;
            }
        }
    }
    reset_hypercube |=
        !std::equal(previous_methods.begin(), previous_methods.end(), methods.begin());
    if (reset_hypercube) {
        set_hypercube(methods);
    }
}

void RegularGridInterpolatorPrivate::clear_target()
{
    target_is_set = false;
    target = std::vector<double>(number_of_axes, 0.);
    results = std::vector<double>(number_of_axes, 0.);
}

void RegularGridInterpolatorPrivate::set_hypercube(std::vector<Method> m_methods)
{
    if (m_methods.size() != number_of_axes) {
        throw BtwxtException(fmt::format("Error setting hypercube. Methods vector does not "
                                         "have the correct number of dimensions."),
                             *logger);
    }
    std::size_t previous_size = hypercube.size();
    std::vector<std::vector<int>> options(number_of_axes, {0, 1});
    reset_hypercube = false;

    hypercube_size_hash = 0;
    std::size_t digit = 1;
    for (std::size_t axis = 0; axis < number_of_axes; axis++) {
        if (target_is_set && floor_to_ceiling_fractions[axis] == 0.0) {
            options[axis] = {0};
            reset_hypercube = true;
        }
        else if (m_methods[axis] == Method::cubic) {
            options[axis] = {-1, 0, 1, 2};
        }
        hypercube_size_hash += options[axis].size() * digit;
        digit *= 10;
    }
    hypercube = {{}};
    for (const auto& list : options) {
        std::vector<std::vector<short>> r;
        for (const auto& x : hypercube) {
            for (const auto item : list) {
                r.push_back(x);
                r.back().push_back(static_cast<short>(item));
            }
        }
        hypercube = std::move(r);
    }
    if (hypercube.size() != previous_size) {
        hypercube_grid_point_data.resize(hypercube.size(),
                                         std::vector<double>(number_of_grid_point_data_sets));
        hypercube_weights.resize(hypercube.size());
    }
}

void RegularGridInterpolatorPrivate::calculate_interpolation_coefficients()
{
    static constexpr std::size_t floor = 0;
    static constexpr std::size_t ceiling = 1;
    for (std::size_t axis = 0; axis < number_of_axes; axis++) {
        double mu = floor_to_ceiling_fractions[axis];
        if (methods[axis] == Method::cubic) {
            interpolation_coefficients[axis][floor] = 2 * mu * mu * mu - 3 * mu * mu + 1;
            interpolation_coefficients[axis][ceiling] = -2 * mu * mu * mu + 3 * mu * mu;
            cubic_slope_coefficients[axis][floor] =
                (mu * mu * mu - 2 * mu * mu + mu) *
                get_axis_cubic_spacing_ratios(axis, floor)[floor_grid_point_coordinates[axis]];
            cubic_slope_coefficients[axis][ceiling] =
                (mu * mu * mu - mu * mu) *
                get_axis_cubic_spacing_ratios(axis, ceiling)[floor_grid_point_coordinates[axis]];
        }
        else {
            if (methods[axis] == Method::constant) {
                mu = mu < 0 ? 0 : 1;
            }
            interpolation_coefficients[axis][floor] = 1 - mu;
            interpolation_coefficients[axis][ceiling] = mu;
            cubic_slope_coefficients[axis][floor] = 0.0;
            cubic_slope_coefficients[axis][ceiling] = 0.0;
        }
        weighting_factors[axis][0] =
            -cubic_slope_coefficients[axis][floor]; // point below floor (-1)
        weighting_factors[axis][1] = interpolation_coefficients[axis][floor] -
                                     cubic_slope_coefficients[axis][ceiling]; // floor (0)
        weighting_factors[axis][2] = interpolation_coefficients[axis][ceiling] +
                                     cubic_slope_coefficients[axis][floor]; // ceiling (1)
        weighting_factors[axis][3] =
            cubic_slope_coefficients[axis][ceiling]; // point above ceiling (2)
    }
}

void RegularGridInterpolatorPrivate::set_hypercube_grid_point_data()
{
    if (results.size() != number_of_grid_point_data_sets) {
        results.resize(number_of_grid_point_data_sets);
        hypercube_grid_point_data.resize(hypercube.size(),
                                         std::vector<double>(number_of_grid_point_data_sets));
        hypercube_cache.clear();
    }
    if (hypercube_cache.count({floor_grid_point_index, hypercube_size_hash})) {
        hypercube_grid_point_data =
            hypercube_cache.at({floor_grid_point_index, hypercube_size_hash});
        return;
    }
    std::size_t hypercube_index = 0;
    for (const auto& v : hypercube) {
        hypercube_grid_point_data[hypercube_index] =
            get_grid_point_data_relative(floor_grid_point_coordinates, v);
        ++hypercube_index;
    }
    hypercube_cache[{floor_grid_point_index, hypercube_size_hash}] = hypercube_grid_point_data;
}

double RegularGridInterpolatorPrivate::get_grid_point_weighting_factor(const std::vector<short>& v)
{
    double weighting_factor = 1.0;
    for (std::size_t axis = 0; axis < number_of_axes; axis++) {
        weighting_factor *= weighting_factors[axis][v[axis] + 1];
    }
    return weighting_factor;
}

const std::vector<double>& RegularGridInterpolatorPrivate::get_target() const
{
    if (!target_is_set) {
        logger->warning(
            fmt::format("The current target was requested, but no target has been set."));
    }
    return target;
}

// Internal getter methods
std::vector<std::vector<short>>& RegularGridInterpolatorPrivate::get_hypercube()
{
    consolidate_methods();
    return hypercube;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_extrapolation_methods() const
{
    std::vector<Method> extrapolation_methods(number_of_axes);
    for (std::size_t axis = 0; axis < number_of_axes; axis++) {
        extrapolation_methods[axis] = grid_axes[axis].get_extrapolation_method();
    }
    return extrapolation_methods;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_interpolation_methods() const
{
    std::vector<Method> interpolation_methods(number_of_axes);
    for (std::size_t axis = 0; axis < number_of_axes; axis++) {
        interpolation_methods[axis] = grid_axes[axis].get_interpolation_method();
    }
    return interpolation_methods;
}

const std::vector<double>& RegularGridInterpolatorPrivate::get_axis_cubic_spacing_ratios(
    const std::size_t axis, const std::size_t floor_or_ceiling) const
{
    return grid_axes[axis].get_cubic_spacing_ratios(floor_or_ceiling);
}

} // namespace Btwxt