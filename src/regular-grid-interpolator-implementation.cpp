/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <sstream>
#include <unordered_map>
#include <cassert>

#include <btwxt/btwxt.h>

#include "regular-grid-interpolator-implementation.h"

namespace Btwxt {

RegularGridInterpolatorImplementation::RegularGridInterpolatorImplementation(
    const std::vector<GridAxis>& grid,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : RegularGridInterpolatorImplementation(grid, {}, std::move(name), courier)
{
}

RegularGridInterpolatorImplementation::RegularGridInterpolatorImplementation(
    const std::vector<GridAxis>& grid_axes,
    const std::vector<GridPointDataSet>& grid_point_data_sets,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : Courier::Sender(std::move(name), courier)
    , grid_axes(grid_axes)
    , grid_point_data_sets(grid_point_data_sets)
    , number_of_grid_point_data_sets(grid_point_data_sets.size())
    , number_of_grid_axes(grid_axes.size())
    , grid_axis_lengths(number_of_grid_axes)
    , grid_axis_step_size(number_of_grid_axes)
    , temporary_coordinates(number_of_grid_axes)
    , temporary_grid_point_data(number_of_grid_point_data_sets, 0.)
    , target(number_of_grid_axes, 0.)
    , floor_grid_point_coordinates(number_of_grid_axes, 0)
    , floor_to_ceiling_fractions(number_of_grid_axes, 0.)
    , target_bounds_status(number_of_grid_axes)
    , methods(number_of_grid_axes, Method::undefined)
    , weighting_factors(number_of_grid_axes, std::vector<double>(4, 0.))
    , results(number_of_grid_point_data_sets, 0.)
    , interpolation_coefficients(number_of_grid_axes, std::vector<double>(2, 0.))
    , cubic_slope_coefficients(number_of_grid_axes, std::vector<double>(2, 0.))
{
    class_name = "RegularGridInterpolator";
    setup();
}

RegularGridInterpolatorImplementation::RegularGridInterpolatorImplementation(
    const RegularGridInterpolatorImplementation& source)
    : Sender(source)
{
    *this = source;
    this->set_axes_parent_pointers();
}

std::size_t RegularGridInterpolatorImplementation::add_grid_point_data_set(
    const GridPointDataSet& grid_point_data_set)
{
    check_grid_point_data_set_size(grid_point_data_set);
    grid_point_data_sets.emplace_back(grid_point_data_set);
    number_of_grid_point_data_sets++;
    temporary_grid_point_data.resize(number_of_grid_point_data_sets);
    results.resize(number_of_grid_point_data_sets);
    hypercube_grid_point_data.resize(hypercube.size(),
                                     std::vector<double>(number_of_grid_point_data_sets));
    hypercube_cache.clear();
    if (target_is_set) {
        set_results();
    }
    return number_of_grid_point_data_sets - 1; // Returns index of new data set
}

void RegularGridInterpolatorImplementation::set_target(const std::vector<double>& target_in)
{
    if (target_in.size() != number_of_grid_axes) {
        send_error(
            fmt::format("Target (size={}) and grid (size={}) do not have the same dimensions.",
                        target_in.size(),
                        number_of_grid_axes));
    }
    if (target_is_set) {
        if ((target_in == target) && (methods == get_interpolation_methods())) {
            return;
        }
    }
    target = target_in;
    target_is_set = true;
    set_floor_grid_point_coordinates();
    calculate_floor_to_ceiling_fractions();
    consolidate_methods();
    calculate_interpolation_coefficients();
    set_results();
}

const std::vector<double>& RegularGridInterpolatorImplementation::get_target() const
{
    if (!target_is_set) {
        send_error("The current target was requested, but no target has been set.");
    }
    return target;
}

void RegularGridInterpolatorImplementation::clear_target()
{
    target_is_set = false;
    target = std::vector<double>(number_of_grid_axes, 0.);
    results = std::vector<double>(number_of_grid_axes, 0.);
}

std::vector<double> RegularGridInterpolatorImplementation::get_results() const
{
    if (number_of_grid_point_data_sets == 0u) {
        send_error("There are no grid point data sets. No results returned.");
    }
    if (!target_is_set) {
        send_error("Results were requested, but no target has been set.");
    }
    return results;
}

std::vector<double>
RegularGridInterpolatorImplementation::get_results(const std::vector<double>& target_in)
{
    set_target(target_in);
    return get_results();
}

void RegularGridInterpolatorImplementation::normalize_grid_point_data_sets_at_target(
    const double scalar)
{
    if (!target_is_set) {
        send_error("Cannot normalize grid point data sets. No target has been set.");
    }
    for (std::size_t data_set_index = 0; data_set_index < number_of_grid_point_data_sets;
         ++data_set_index) {
        normalize_grid_point_data_set(data_set_index, results[data_set_index] * scalar);
    }
    hypercube_cache.clear();
    set_results();
}

double RegularGridInterpolatorImplementation::normalize_grid_point_data_set_at_target(
    std::size_t data_set_index, double scalar)
{
    check_data_set_index(data_set_index, "normalize grid point data set");
    if (!target_is_set) {
        send_error(fmt::format(
            "GridPointDataSet '{}': Cannot normalize grid point data set. No target has been set.",
            grid_point_data_sets[data_set_index].name));
    }
    // create a scalar which represents the product of the inverted normalization factor and the
    // value in the data set at the independent variable reference value
    double total_scalar = results[data_set_index] * scalar;
    normalize_grid_point_data_set(data_set_index, total_scalar);
    hypercube_cache.clear();
    set_results();

    return total_scalar;
}

void RegularGridInterpolatorImplementation::normalize_grid_point_data_set(
    std::size_t data_set_index, double scalar)
{
    check_data_set_index(data_set_index, "normalize grid point data set");
    auto& data_set = grid_point_data_sets[data_set_index].data;
    if (scalar == 0.0) {
        send_error(
            fmt::format("GridPointDataSet '{}': Attempt to normalize grid point data set by zero.",
                        grid_point_data_sets[data_set_index].name));
    }
    scalar = 1.0 / scalar;
    std::transform(data_set.begin(),
                   data_set.end(),
                   data_set.begin(),
                   [scalar](double x) -> double { return x * scalar; });
}

std::string RegularGridInterpolatorImplementation::write_data()
{
    std::stringstream output("");

    std::vector<std::vector<double>> grid_axes_values;

    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; ++axis_index) {
        output << grid_axes[axis_index].name << ",";
        grid_axes_values.push_back(grid_axes[axis_index].get_values());
    }

    std::vector<std::vector<double>> grid_points = cartesian_product(grid_axes_values);

    for (std::size_t data_set_index = 0; data_set_index < number_of_grid_point_data_sets;
         ++data_set_index) {
        output << grid_point_data_sets[data_set_index].name << ",";
    }
    output << std::endl;

    for (std::size_t grid_point_index = 0; grid_point_index < number_of_grid_points;
         ++grid_point_index) {
        for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; ++axis_index) {
            output << grid_points[grid_point_index][axis_index] << ",";
        }
        for (std::size_t data_set_index = 0; data_set_index < number_of_grid_point_data_sets;
             ++data_set_index) {
            output << grid_point_data_sets[data_set_index].data[grid_point_index] << ",";
        }
        output << std::endl;
    }
    return output.str();
}

void RegularGridInterpolatorImplementation::set_courier(
    const std::shared_ptr<Courier::Courier>& courier_in, bool set_grid_axes_couriers)
{
    courier = courier_in;
    if (set_grid_axes_couriers) {
        for (auto& axis : grid_axes) {
            axis.set_courier(courier);
        }
    }
}

const std::vector<double>&
RegularGridInterpolatorImplementation::get_grid_point_data(std::size_t grid_point_index)
{
    for (std::size_t i = 0; i < number_of_grid_point_data_sets; ++i) {
        temporary_grid_point_data[i] = grid_point_data_sets[i].data[grid_point_index];
    }
    return temporary_grid_point_data;
}

const std::vector<double>&
RegularGridInterpolatorImplementation::get_grid_point_data(const std::vector<std::size_t>& coords)
{
    return get_grid_point_data(get_grid_point_index(coords));
}

std::vector<double> RegularGridInterpolatorImplementation::get_grid_point_data_relative(
    const std::vector<std::size_t>& coords, const std::vector<short>& translation)
{
    return get_grid_point_data(get_grid_point_index_relative(coords, translation));
}

// Internal getter methods
std::vector<Method> RegularGridInterpolatorImplementation::get_interpolation_methods() const
{
    std::vector<Method> interpolation_methods(number_of_grid_axes);
    static const std::unordered_map<InterpolationMethod, Method> interpolation_method_map {
        {InterpolationMethod::linear, Method::linear}, {InterpolationMethod::cubic, Method::cubic}};

    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index++) {
        interpolation_methods[axis_index] =
            interpolation_method_map.at(grid_axes[axis_index].get_interpolation_method());
    }
    return interpolation_methods;
}

std::vector<Method> RegularGridInterpolatorImplementation::get_extrapolation_methods() const
{
    std::vector<Method> extrapolation_methods(number_of_grid_axes);
    static const std::unordered_map<ExtrapolationMethod, Method> extrapolation_method_map {
        {ExtrapolationMethod::constant, Method::constant},
        {ExtrapolationMethod::linear, Method::linear}};
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index++) {
        extrapolation_methods[axis_index] =
            extrapolation_method_map.at(grid_axes[axis_index].get_extrapolation_method());
    }
    return extrapolation_methods;
}

std::size_t RegularGridInterpolatorImplementation::get_grid_point_index(
    const std::vector<std::size_t>& coords) const
{
    std::size_t grid_point_index = 0;
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; ++axis_index) {
        grid_point_index += coords[axis_index] * grid_axis_step_size[axis_index];
    }
    return grid_point_index;
}

double RegularGridInterpolatorImplementation::get_grid_point_weighting_factor(
    const std::vector<short>& hypercube_indices)
{
    double weighting_factor = 1.0;
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index++) {
        weighting_factor *= weighting_factors[axis_index][hypercube_indices[axis_index] + 1];
    }
    return weighting_factor;
}

std::vector<std::size_t> RegularGridInterpolatorImplementation::get_neighboring_indices_at_target()
{
    if (!target_is_set) {
        send_error("Cannot retrieve neighboring indices. No target has been set.");
    }
    std::vector<std::vector<std::size_t>> axes_neighbor_indices(
        number_of_grid_axes,
        std::vector<std::size_t>()); // For each axis, what are the neighboring indices?
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; ++axis_index) {
        auto floor_index = floor_grid_point_coordinates[axis_index];
        if (floor_to_ceiling_fractions[axis_index] < 1.0) {
            axes_neighbor_indices[axis_index].push_back(floor_index);
        }
        if (grid_axis_lengths[axis_index] > 1 && floor_to_ceiling_fractions[axis_index] > 0.0) {
            axes_neighbor_indices[axis_index].push_back(floor_index + 1);
        }
    }
    std::vector<std::vector<std::size_t>> axes_neighbor_coordinates =
        cartesian_product(axes_neighbor_indices);
    std::vector<std::size_t> neighbor_indices;
    neighbor_indices.reserve(axes_neighbor_coordinates.size());
    for (const auto& coordinates : axes_neighbor_coordinates) {
        neighbor_indices.push_back(get_grid_point_index(coordinates));
    }
    return neighbor_indices;
}

std::vector<std::size_t> RegularGridInterpolatorImplementation::get_neighboring_indices_at_target(
    const std::vector<double>& target_in)
{
    set_target(target_in);
    return get_neighboring_indices_at_target();
}
// private methods

void RegularGridInterpolatorImplementation::setup()
{
    // set axis sizes and calculate number of grid points
    number_of_grid_points = 1;
    for (std::size_t axis_index = number_of_grid_axes; axis_index-- > 0;) {
        std::size_t length =
            grid_axes[axis_index].get_length(); // length > 0 ensured by GridAxis constructor
        grid_axis_lengths[axis_index] = length;
        grid_axis_step_size[axis_index] = number_of_grid_points;
        number_of_grid_points *= length;
    }

    // set parent interpolator pointer
    set_axes_parent_pointers();

    // Check grid point data set sizes
    for (const auto& grid_point_data_set : grid_point_data_sets) {
        check_grid_point_data_set_size(grid_point_data_set);
    }
}

void RegularGridInterpolatorImplementation::set_axes_parent_pointers()
{
    for (auto& grid_axis : grid_axes) {
        grid_axis.parent_pointer = this;
    }
}

void RegularGridInterpolatorImplementation::check_grid_point_data_set_size(
    const GridPointDataSet& grid_point_data_set)
{
    if (grid_point_data_set.data.size() != number_of_grid_points) {
        send_error(fmt::format(
            "GridPointDataSet '{}': Size ({}) does not match number of grid points ({}).",
            grid_point_data_set.name,
            grid_point_data_set.data.size(),
            number_of_grid_points));
    }
}

void RegularGridInterpolatorImplementation::set_results()
{
    set_hypercube_grid_point_data();
    std::fill(results.begin(), results.end(), 0.0);
    for (std::size_t hypercube_index = 0; hypercube_index < hypercube.size(); ++hypercube_index) {
        hypercube_weights[hypercube_index] =
            get_grid_point_weighting_factor(hypercube[hypercube_index]);
        for (std::size_t data_set_index = 0; data_set_index < number_of_grid_point_data_sets;
             ++data_set_index) {
            results[data_set_index] += hypercube_grid_point_data[hypercube_index][data_set_index] *
                                       hypercube_weights[hypercube_index];
        }
    }
}

// Internal calculation methods

std::size_t RegularGridInterpolatorImplementation::get_grid_point_index_relative(
    const std::vector<std::size_t>& coords, const std::vector<short>& translation)
{
    int new_coord;
    for (std::size_t axis_index = 0; axis_index < coords.size(); axis_index++) {
        new_coord = static_cast<int>(coords[axis_index]) + translation[axis_index];
        if (new_coord < 0) {
            temporary_coordinates[axis_index] = 0u;
        }
        else if (new_coord >= static_cast<int>(grid_axis_lengths[axis_index])) {
            temporary_coordinates[axis_index] = grid_axis_lengths[axis_index] - 1u;
        }
        else {
            temporary_coordinates[axis_index] = new_coord;
        }
    }
    return get_grid_point_index(temporary_coordinates);
}

void RegularGridInterpolatorImplementation::set_floor_grid_point_coordinates()
{
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index += 1) {
        set_axis_floor_grid_point_index(axis_index);
    }
    floor_grid_point_index = get_grid_point_index(floor_grid_point_coordinates);
}

void RegularGridInterpolatorImplementation::set_axis_floor_grid_point_index(std::size_t axis_index)
{
    const auto& axis_values = grid_axes[axis_index].get_values();
    int length = static_cast<int>(grid_axis_lengths[axis_index]);
    if (target[axis_index] < get_extrapolation_limits(axis_index).first) {
        target_bounds_status[axis_index] = TargetBoundsStatus::below_lower_extrapolation_limit;
        floor_grid_point_coordinates[axis_index] = 0u;
    }
    else if (target[axis_index] > get_extrapolation_limits(axis_index).second) {
        target_bounds_status[axis_index] = TargetBoundsStatus::above_upper_extrapolation_limit;
        floor_grid_point_coordinates[axis_index] =
            std::max(length - 2,
                     0); // length-2 because that's the left side of the (length-2, length-1) edge.
    }
    else if (target[axis_index] < axis_values[0]) {
        target_bounds_status[axis_index] = TargetBoundsStatus::extrapolate_low;
        floor_grid_point_coordinates[axis_index] = 0;
    }
    else if (target[axis_index] > axis_values.back()) {
        target_bounds_status[axis_index] = TargetBoundsStatus::extrapolate_high;
        floor_grid_point_coordinates[axis_index] =
            std::max(length - 2,
                     0); // length-2 because that's the left side of the (length-2, length-1) edge.
    }
    else if (target[axis_index] == axis_values.back()) {
        target_bounds_status[axis_index] = TargetBoundsStatus::interpolate;
        floor_grid_point_coordinates[axis_index] =
            std::max(length - 2,
                     0); // length-2 because that's the left side of the (length-2, length-1) edge.
    }
    else {
        target_bounds_status[axis_index] = TargetBoundsStatus::interpolate;
        auto upper = std::upper_bound(axis_values.begin(), axis_values.end(), target[axis_index]);
        floor_grid_point_coordinates[axis_index] = upper - axis_values.begin() - 1;
    }
}

void RegularGridInterpolatorImplementation::calculate_floor_to_ceiling_fractions()
{
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; ++axis_index) {
        if (grid_axis_lengths[axis_index] > 1) {
            auto& axis_values = grid_axes[axis_index].get_values();
            auto floor_index = floor_grid_point_coordinates[axis_index];
            floor_to_ceiling_fractions[axis_index] = compute_fraction(
                target[axis_index], axis_values[floor_index], axis_values[floor_index + 1]);
        }
        else {
            floor_to_ceiling_fractions[axis_index] = 1.0;
        }
    }
}

void RegularGridInterpolatorImplementation::consolidate_methods()
// If out of bounds, extrapolate according to prescription
// If outside of extrapolation limits, send a warning and perform constant extrapolation.
{
    previous_methods = methods;
    methods = get_interpolation_methods();
    if (target_is_set) {
        auto extrapolation_methods = get_extrapolation_methods();
        constexpr std::string_view error_format {
            "GridAxis '{}': The target ({:.6g}) is {} the extrapolation "
            "limit ({:.6g})."};
        for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index++) {
            switch (target_bounds_status[axis_index]) {
            case TargetBoundsStatus::extrapolate_low:
            case TargetBoundsStatus::extrapolate_high:
                methods[axis_index] = extrapolation_methods[axis_index];
                break;
            case TargetBoundsStatus::below_lower_extrapolation_limit:
                send_error(fmt::format(error_format,
                                       grid_axes[axis_index].name,
                                       target[axis_index],
                                       "below",
                                       get_extrapolation_limits(axis_index).first));
                break;
            case TargetBoundsStatus::above_upper_extrapolation_limit:
                send_error(fmt::format(error_format,
                                       grid_axes[axis_index].name,
                                       target[axis_index],
                                       "above",
                                       get_extrapolation_limits(axis_index).second));
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

void RegularGridInterpolatorImplementation::set_hypercube(std::vector<Method> methods_in)
{
    assert(methods_in.size() == number_of_grid_axes);
    std::size_t previous_size = hypercube.size();
    std::vector<std::vector<int>> options(number_of_grid_axes, {0, 1});
    reset_hypercube = false;

    hypercube_size_hash = 0;
    std::size_t digit = 1;
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index++) {
        if (target_is_set && floor_to_ceiling_fractions[axis_index] == 0.0) {
            options[axis_index] = {0};
            reset_hypercube = true;
        }
        else if (methods_in[axis_index] == Method::cubic) {
            options[axis_index] = {-1, 0, 1, 2};
        }
        hypercube_size_hash += options[axis_index].size() * digit;
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

void RegularGridInterpolatorImplementation::calculate_interpolation_coefficients()
{
    static constexpr std::size_t floor = 0;
    static constexpr std::size_t ceiling = 1;
    for (std::size_t axis_index = 0; axis_index < number_of_grid_axes; axis_index++) {
        double mu = floor_to_ceiling_fractions[axis_index];
        if (methods[axis_index] == Method::cubic) {
            interpolation_coefficients[axis_index][floor] = 2 * mu * mu * mu - 3 * mu * mu + 1;
            interpolation_coefficients[axis_index][ceiling] = -2 * mu * mu * mu + 3 * mu * mu;
            cubic_slope_coefficients[axis_index][floor] =
                (mu * mu * mu - 2 * mu * mu + mu) *
                get_axis_cubic_spacing_ratios(axis_index,
                                              floor)[floor_grid_point_coordinates[axis_index]];
            cubic_slope_coefficients[axis_index][ceiling] =
                (mu * mu * mu - mu * mu) *
                get_axis_cubic_spacing_ratios(axis_index,
                                              ceiling)[floor_grid_point_coordinates[axis_index]];
        }
        else {
            if (methods[axis_index] == Method::constant) {
                mu = mu < 0 ? 0 : 1;
            }
            interpolation_coefficients[axis_index][floor] = 1 - mu;
            interpolation_coefficients[axis_index][ceiling] = mu;
            cubic_slope_coefficients[axis_index][floor] = 0.0;
            cubic_slope_coefficients[axis_index][ceiling] = 0.0;
        }
        weighting_factors[axis_index][0] =
            -cubic_slope_coefficients[axis_index][floor]; // point below floor (-1)
        weighting_factors[axis_index][1] =
            interpolation_coefficients[axis_index][floor] -
            cubic_slope_coefficients[axis_index][ceiling]; // floor (0)
        weighting_factors[axis_index][2] =
            interpolation_coefficients[axis_index][ceiling] +
            cubic_slope_coefficients[axis_index][floor]; // ceiling (1)
        weighting_factors[axis_index][3] =
            cubic_slope_coefficients[axis_index][ceiling]; // point above ceiling (2)
    }
}

void RegularGridInterpolatorImplementation::set_hypercube_grid_point_data()
{
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
} // namespace Btwxt