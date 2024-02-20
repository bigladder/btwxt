/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>
#include <numeric>

// btwxt
#include <btwxt/btwxt.h>

// btwxt private
#include "regular-grid-interpolator-implementation.h"

namespace Btwxt {

std::vector<GridAxis> construct_grid_axes(const std::vector<std::vector<double>>& grid_axis_vectors,
                                          const std::shared_ptr<Courier::Courier>& courier_in)
{
    std::vector<GridAxis> grid_axes;
    grid_axes.reserve(grid_axis_vectors.size());
    for (const auto& axis : grid_axis_vectors) {
        grid_axes.emplace_back(axis,
                               InterpolationMethod::linear,
                               ExtrapolationMethod::constant,
                               std::pair<double, double> {-DBL_MAX, DBL_MAX},
                               fmt::format("Axis {}", grid_axes.size() + 1),
                               courier_in);
    }
    return grid_axes;
}

std::vector<GridPointDataSet>
construct_grid_point_data_sets(const std::vector<std::vector<double>>& grid_point_data_vectors)
{
    std::vector<GridPointDataSet> grid_point_data_sets;
    grid_point_data_sets.reserve(grid_point_data_vectors.size());
    for (const auto& grid_point_data_set : grid_point_data_vectors) {
        grid_point_data_sets.emplace_back(
            grid_point_data_set, fmt::format("Data Set {}", grid_point_data_sets.size() + 1));
    }
    return grid_point_data_sets;
}

// Constructors

RegularGridInterpolator::RegularGridInterpolator() = default;

RegularGridInterpolator::RegularGridInterpolator(
    const std::vector<std::vector<double>>& grid_axis_vectors,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : RegularGridInterpolator(construct_grid_axes(grid_axis_vectors, courier),
                              std::vector<GridPointDataSet>(),
                              std::move(name),
                              courier)
{
}

RegularGridInterpolator::RegularGridInterpolator(
    const std::vector<std::vector<double>>& grid_axis_vectors,
    const std::vector<std::vector<double>>& grid_point_data_vectors,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : RegularGridInterpolator(construct_grid_axes(grid_axis_vectors, courier),
                              construct_grid_point_data_sets(grid_point_data_vectors),
                              std::move(name),
                              courier)
{
}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis>& grid,
                                                 std::string name,
                                                 const std::shared_ptr<Courier::Courier>& courier)
    : implementation(
          std::make_unique<RegularGridInterpolatorImplementation>(grid, std::move(name), courier))
{
}

RegularGridInterpolator::RegularGridInterpolator(
    const std::vector<GridAxis>& grid_axes,
    const std::vector<std::vector<double>>& grid_point_data_vectors,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : implementation(std::make_unique<RegularGridInterpolatorImplementation>(
          grid_axes,
          construct_grid_point_data_sets(grid_point_data_vectors),
          std::move(name),
          courier))
{
}

RegularGridInterpolator::RegularGridInterpolator(
    const std::vector<std::vector<double>>& grid_axis_vectors,
    const std::vector<GridPointDataSet>& grid_point_data_sets,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : implementation(std::make_unique<RegularGridInterpolatorImplementation>(
          construct_grid_axes(grid_axis_vectors, courier),
          grid_point_data_sets,
          std::move(name),
          courier))
{
}

RegularGridInterpolator::RegularGridInterpolator(
    const std::vector<GridAxis>& grid_axes,
    const std::vector<GridPointDataSet>& grid_point_data_sets,
    std::string name,
    const std::shared_ptr<Courier::Courier>& courier)
    : implementation(std::make_unique<RegularGridInterpolatorImplementation>(
          grid_axes, grid_point_data_sets, std::move(name), courier))
{
}

RegularGridInterpolator::~RegularGridInterpolator() = default;

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator& source)
{
    *this = source;
    this->implementation =
        source.implementation
            ? std::make_unique<RegularGridInterpolatorImplementation>(*source.implementation)
            : nullptr;
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator& source,
                                                 const std::shared_ptr<Courier::Courier>& courier)
    : RegularGridInterpolator(source)
{
    this->implementation->set_courier(courier);
}

RegularGridInterpolator& RegularGridInterpolator::operator=(const RegularGridInterpolator& source)
{
    implementation =
        source.implementation
            ? std::make_unique<RegularGridInterpolatorImplementation>(*(source.implementation))
            : nullptr;
    return *this;
}

// Public manipulation methods

std::size_t
RegularGridInterpolator::add_grid_point_data_set(const std::vector<double>& grid_point_data_vector,
                                                 const std::string& name)
{
    std::string resolved_name {name};
    if (resolved_name.empty()) {
        resolved_name =
            fmt::format("Data Set {}", implementation->get_number_of_grid_point_data_sets());
    }
    return add_grid_point_data_set(GridPointDataSet(grid_point_data_vector, resolved_name));
}

std::size_t
RegularGridInterpolator::add_grid_point_data_set(const GridPointDataSet& grid_point_data_set)
{
    return implementation->add_grid_point_data_set(grid_point_data_set);
}

void RegularGridInterpolator::set_axis_extrapolation_method(const std::size_t axis_index,
                                                            const ExtrapolationMethod method)
{
    implementation->set_axis_extrapolation_method(axis_index, method);
}

void RegularGridInterpolator::set_axis_interpolation_method(const std::size_t axis_index,
                                                            const InterpolationMethod method)
{
    implementation->set_axis_interpolation_method(axis_index, method);
}

void RegularGridInterpolator::set_axis_extrapolation_limits(
    const std::size_t axis_index, const std::pair<double, double>& extrapolation_limits)
{
    implementation->set_axis_extrapolation_limits(axis_index, extrapolation_limits);
}

std::size_t RegularGridInterpolator::get_number_of_dimensions()
{
    return implementation->get_number_of_grid_axes();
}

// Public normalization methods
double RegularGridInterpolator::normalize_grid_point_data_set_at_target(
    std::size_t data_set_index, const std::vector<double>& target, const double scalar)
{
    set_target(target);
    return normalize_grid_point_data_set_at_target(data_set_index, scalar);
}

double RegularGridInterpolator::normalize_grid_point_data_set_at_target(std::size_t data_set_index,
                                                                        const double scalar)
{
    return implementation->normalize_grid_point_data_set_at_target(data_set_index, scalar);
}

void RegularGridInterpolator::normalize_grid_point_data_sets_at_target(
    const std::vector<double>& target, const double scalar)
{
    set_target(target);
    normalize_grid_point_data_sets_at_target(scalar);
}

void RegularGridInterpolator::normalize_grid_point_data_sets_at_target(const double scalar)
{
    return implementation->normalize_grid_point_data_sets_at_target(scalar);
}

// Public printing methods
std::string RegularGridInterpolator::write_data() { return implementation->write_data(); }

// Public calculation methods
void RegularGridInterpolator::set_target(const std::vector<double>& target)
{
    implementation->set_target(target);
}

double RegularGridInterpolator::get_value_at_target(const std::vector<double>& target,
                                                    std::size_t data_set_index)
{
    set_target(target);
    return get_value_at_target(data_set_index);
}

double RegularGridInterpolator::get_value_at_target(std::size_t data_set_index)
{
    return implementation->get_results()[data_set_index];
}

std::vector<double> RegularGridInterpolator::get_values_at_target(const std::vector<double>& target)
{
    return implementation->get_results(target);
}

std::vector<double> RegularGridInterpolator::get_values_at_target()
{
    return implementation->get_results();
}

const std::vector<double>& RegularGridInterpolator::get_target()
{
    return implementation->get_target();
}

const std::vector<TargetBoundsStatus>& RegularGridInterpolator::get_target_bounds_status() const
{
    return implementation->get_target_bounds_status();
}

void RegularGridInterpolator::clear_target() { implementation->clear_target(); }

// Public logging

void RegularGridInterpolator::set_courier(const std::shared_ptr<Courier::Courier>& courier,
                                          bool set_grid_axes_couriers)
{
    implementation->set_courier(courier, set_grid_axes_couriers);
}

std::shared_ptr<Courier::Courier> RegularGridInterpolator::get_courier()
{
    return implementation->get_courier();
}

} // namespace Btwxt
