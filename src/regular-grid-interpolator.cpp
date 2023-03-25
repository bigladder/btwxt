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

// Constructors
RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>> &grid,
                                                 const std::vector<std::vector<double>> &values,
                                                 const std::shared_ptr<Courierr::Courierr> &logger)
    : regular_grid_interpolator(
          std::make_unique<RegularGridInterpolatorPrivate>(grid, values, logger)) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<std::vector<double>> &grid,
                                                 std::shared_ptr<Courierr::Courierr> logger)
    : RegularGridInterpolator(grid, {}, std::move(logger)) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis> &grid,
                                                 const std::vector<std::vector<double>> &values,
                                                 const std::shared_ptr<Courierr::Courierr> &logger)
    : regular_grid_interpolator(
          std::make_unique<RegularGridInterpolatorPrivate>(grid, values, logger)) {}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<GridAxis> &grid, const std::vector<std::vector<double>> &values,
    std::shared_ptr<Courierr::Courierr> logger)
    : grid_axes(grid),
      value_tables(values),
      number_of_tables(values.size()),
      number_of_axes(grid.size()),
      axis_lengths(number_of_axes),
      axis_step_size(number_of_axes),
      temporary_coordinates(number_of_axes),
      grid_point_values(number_of_tables, 0.),
      target(number_of_axes, 0.),
      floor_grid_point_coordinates(number_of_axes, 0),
      floor_to_ceiling_fractions(number_of_axes, 0.),
      is_inbounds(number_of_axes),
      methods(number_of_axes, Method::undefined),
      weighting_factors(number_of_axes, std::vector<double>(4, 0.)),
      results(number_of_tables, 0.),
      interpolation_coefficients(number_of_axes, std::vector<double>(2, 0.)),
      cubic_slope_coefficients(number_of_axes, std::vector<double>(2, 0.)),
      logger(std::move(logger)) {
  set_axis_sizes();
}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<std::vector<double>> &grid, const std::vector<std::vector<double>> &values,
    std::shared_ptr<Courierr::Courierr> logger)
    : RegularGridInterpolatorPrivate(construct_axes(grid, logger), values, logger) {}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<std::vector<double>> &grid, std::shared_ptr<Courierr::Courierr> logger)
    : RegularGridInterpolatorPrivate(construct_axes(grid, logger), {}, logger) {}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<GridAxis> &grid, std::shared_ptr<Courierr::Courierr> logger)
    : RegularGridInterpolatorPrivate(grid, {}, logger) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis> &grid,
                                                 std::shared_ptr<Courierr::Courierr> logger)
    : regular_grid_interpolator(std::make_unique<RegularGridInterpolatorPrivate>(grid, logger)) {}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source) {
  *this = source;
  this->regular_grid_interpolator =
      std::make_unique<RegularGridInterpolatorPrivate>(*source.regular_grid_interpolator);
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source,
                                                 std::shared_ptr<Courierr::Courierr> logger) {
  *this = source;
  this->regular_grid_interpolator =
      std::make_unique<RegularGridInterpolatorPrivate>(*source.regular_grid_interpolator);
  this->regular_grid_interpolator->logger = logger;
}

RegularGridInterpolator &RegularGridInterpolator::operator=(const RegularGridInterpolator &source) {
  regular_grid_interpolator =
      std::make_unique<RegularGridInterpolatorPrivate>(*(source.regular_grid_interpolator));
  return *this;
}

void RegularGridInterpolatorPrivate::set_axis_sizes() {
  number_of_values = 1;
  for (std::size_t axis = number_of_axes - 1; /* axis >= 0 */ axis < number_of_axes; --axis) {
    std::size_t length = grid_axes[axis].get_length();
    if (length == 0) {
      throw BtwxtException(fmt::format("Axis {} has length zero.", axis), *logger);
    }
    axis_lengths[axis] = length;
    axis_step_size[axis] = number_of_values;
    number_of_values *= length;
  }
}

// Public manipulation methods

std::size_t RegularGridInterpolator::add_value_table(const std::vector<double> &value_vector) {
  return regular_grid_interpolator->add_value_table(value_vector);
}

std::size_t
RegularGridInterpolatorPrivate::add_value_table(const std::vector<double> &value_vector) {
  if (value_vector.size() != number_of_values) {
    throw BtwxtException(fmt::format("Input value table does not match the values size: {} != {}",
                                     value_vector.size(), number_of_values),
                         *logger);
  }
  value_tables.push_back(value_vector);
  number_of_tables++;
  grid_point_values.resize(number_of_tables);
  results.resize(number_of_tables);
  return number_of_tables - 1; // Returns index of new table
}

void RegularGridInterpolator::set_axis_extrapolation_method(const std::size_t axis,
                                                            const Method method) {
  regular_grid_interpolator->set_axis_extrapolation_method(axis, method);
}

void RegularGridInterpolator::set_axis_interpolation_method(const std::size_t axis,
                                                            const Method method) {
  regular_grid_interpolator->set_axis_interpolation_method(axis, method);
}

void RegularGridInterpolator::set_axis_extrapolation_limits(
    const std::size_t axis, const std::pair<double, double> &extrap_limits) {
  regular_grid_interpolator->set_axis_extrapolation_limits(axis, extrap_limits);
}

// Public normalization methods
double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const std::vector<double> &target,
                                                           const double scalar) {
  set_target(target);
  return normalize_values_at_target(table_index, scalar);
}

double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const double scalar) {
  return regular_grid_interpolator->normalize_grid_values_at_target(table_index, scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const std::vector<double> &target,
                                                         const double scalar) {
  set_target(target);
  normalize_values_at_target(scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const double scalar) {
  return regular_grid_interpolator->normalize_grid_values_at_target(scalar);
}

void RegularGridInterpolatorPrivate::normalize_grid_values_at_target(const double scalar) {
  if (!target_is_set) {
    throw BtwxtException(fmt::format("Cannot normalize grid values. No target has been set."),
                         *logger);
  }
  for (std::size_t table_index = 0; table_index < number_of_tables; ++table_index) {
    normalize_value_table(table_index, results[table_index] * scalar);
  }
  hypercube_cache.clear();
  set_results();
}

double RegularGridInterpolatorPrivate::normalize_grid_values_at_target(std::size_t table_num,
                                                                       const double scalar) {
  if (!target_is_set) {
    throw BtwxtException(fmt::format("Cannot normalize grid values. No target has been set."),
                         *logger);
  }
  // create a scalar which represents the product of the inverted normalization factor and the
  // value in the table at the independent variable reference value
  double total_scalar = results[table_num] * scalar;
  normalize_value_table(table_num, total_scalar);
  hypercube_cache.clear();
  set_results();

  return total_scalar;
}

void RegularGridInterpolatorPrivate::normalize_value_table(std::size_t table_num, double scalar) {
  auto &table = value_tables[table_num];
  if (scalar == 0.0) {
    throw BtwxtException("Attempt to normalize values by zero.", *logger);
  }
  scalar = 1.0 / scalar;
  std::transform(table.begin(), table.end(), table.begin(),
                 std::bind(std::multiplies<double>(), std::placeholders::_1, scalar));
}

// Public getter methods

std::pair<double, double>
RegularGridInterpolator::get_axis_extrapolation_limits(const std::size_t axis) {
  return regular_grid_interpolator->get_extrapolation_limits(axis);
}

std::pair<double, double>
RegularGridInterpolatorPrivate::get_extrapolation_limits(std::size_t axis) const {
  return grid_axes[axis].get_extrapolation_limits();
}

const std::vector<double> &RegularGridInterpolatorPrivate::get_axis_values(const std::size_t axis) {
  return grid_axes[axis].get_values();
}

// Public printing methods
std::string RegularGridInterpolator::write_data() {
  return regular_grid_interpolator->write_data();
}

std::string RegularGridInterpolatorPrivate::write_data() {
  std::vector<std::size_t> indices(number_of_axes, 0);
  std::stringstream output("");

  for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
    output << "Axis " << axis + 1 << ",";
  }
  for (std::size_t tab = 0; tab < number_of_tables; ++tab) {
    output << "Value " << tab + 1 << ",";
  }
  output << std::endl;
  for (std::size_t index = 0; index < number_of_values; ++index) {
    for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
      output << get_axis_values(axis)[indices[axis]] << ",";
    }
    for (std::size_t tab = 0; tab < number_of_tables; ++tab) {
      output << value_tables[tab][index] << ",";
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
void RegularGridInterpolator::set_target(const std::vector<double> &target) {
  regular_grid_interpolator->set_target(target);
}

void RegularGridInterpolatorPrivate::set_target(const std::vector<double> &v) {
  if (v.size() != number_of_axes) {
    throw BtwxtException(fmt::format("Target and grid do not have the same dimensions."), *logger);
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

void RegularGridInterpolatorPrivate::set_results() {
  set_hypercube_values();
  std::fill(results.begin(), results.end(), 0.0);
  for (std::size_t i = 0; i < hypercube.size(); ++i) {
    hypercube_weights[i] = get_grid_point_weighting_factor(hypercube[i]);
    const auto &values = hypercube_values[i];
    for (std::size_t j = 0; j < number_of_tables; ++j) {
      results[j] += values[j] * hypercube_weights[i];
    }
  }
}

double RegularGridInterpolator::get_value_at_target(const std::vector<double> &target,
                                                    std::size_t table_index) {
  set_target(target);
  return get_value_at_target(table_index);
}

double RegularGridInterpolator::get_value_at_target(std::size_t table_index) {
  return regular_grid_interpolator->get_results()[table_index];
}

std::vector<double> RegularGridInterpolatorPrivate::get_results() {
  if (number_of_tables == 0u) {
    logger->warning(
        fmt::format("There are no value tables in the gridded data. No results returned."));
  }
  if (!target_is_set) {
    logger->warning(fmt::format("Results were requested, but no target has been set."));
  }
  return results;
}

std::vector<double>
RegularGridInterpolatorPrivate::get_results(const std::vector<double> &target_in) {
  set_target(target_in);
  return get_results();
}

std::vector<double>
RegularGridInterpolator::get_values_at_target(const std::vector<double> &target) {
  return regular_grid_interpolator->get_results(target);
}

std::vector<double> RegularGridInterpolator::get_values_at_target() {
  return regular_grid_interpolator->get_results();
}

std::vector<double> RegularGridInterpolator::get_target() {
  return regular_grid_interpolator->get_target();
}

void RegularGridInterpolator::clear_target() { regular_grid_interpolator->clear_target(); }

// Public logging

void RegularGridInterpolator::set_logger(std::shared_ptr<Courierr::Courierr> logger,
                                         bool set_grid_axes_loggers) {
  regular_grid_interpolator->set_logger(logger, set_grid_axes_loggers);
}

void RegularGridInterpolatorPrivate::set_logger(std::shared_ptr<Courierr::Courierr> logger_in,
                                                bool set_grid_axes_loggers) {
  logger = logger_in;
  if (set_grid_axes_loggers) {
    for (auto &axis : grid_axes) {
      axis.set_logger(logger);
    }
  }
}

std::shared_ptr<Courierr::Courierr> RegularGridInterpolator::get_logger() {
  return regular_grid_interpolator->logger;
}

// Internal calculation methods

std::size_t
RegularGridInterpolatorPrivate::get_grid_point_index(const std::vector<std::size_t> &coords) const {
  std::size_t index = 0;
  for (std::size_t axis = 0; axis < number_of_axes; ++axis) {
    index += coords[axis] * axis_step_size[axis];
  }
  return index;
}

const std::vector<double> &
RegularGridInterpolatorPrivate::get_grid_point_values(const std::size_t index) {
  for (std::size_t i = 0; i < number_of_tables; ++i) {
    grid_point_values[i] = value_tables[i][index];
  }
  return grid_point_values;
}

const std::vector<double> &
RegularGridInterpolatorPrivate::get_grid_point_values(const std::vector<std::size_t> &coords) {
  std::size_t index = get_grid_point_index(coords);
  return get_grid_point_values(index);
}

std::size_t RegularGridInterpolatorPrivate::get_grid_point_index_relative(
    const std::vector<std::size_t> &coords, const std::vector<short> &translation) {
  int new_coord;
  for (std::size_t axis = 0; axis < coords.size(); axis++) {
    new_coord = static_cast<int>(coords[axis]) + translation[axis];
    if (new_coord < 0) {
      temporary_coordinates[axis] = 0u;
    } else if (new_coord >= static_cast<int>(axis_lengths[axis])) {
      temporary_coordinates[axis] = axis_lengths[axis] - 1u;
    } else {
      temporary_coordinates[axis] = new_coord;
    }
  }
  return get_grid_point_index(temporary_coordinates);
}

std::vector<double> RegularGridInterpolatorPrivate::get_grid_point_values_relative(
    const std::vector<std::size_t> &coords, const std::vector<short> &translation) {
  return get_grid_point_values(get_grid_point_index_relative(coords, translation));
}

void RegularGridInterpolatorPrivate::set_floor_grid_point_coordinates() {
  for (std::size_t dim = 0; dim < number_of_axes; dim += 1) {
    set_axis_floor_grid_point_index(dim);
  }
  floor_grid_point_index = get_grid_point_index(floor_grid_point_coordinates);
}

void RegularGridInterpolatorPrivate::set_axis_floor_grid_point_index(size_t axis) {
  const auto &axis_values = get_axis_values(axis);
  int length = static_cast<int>(axis_lengths[axis]);
  if (target[axis] < get_extrapolation_limits(axis).first) {
    is_inbounds[axis] = Bounds::outlaw;
    floor_grid_point_coordinates[axis] = 0u;
  } else if (target[axis] > get_extrapolation_limits(axis).second) {
    is_inbounds[axis] = Bounds::outlaw;
    floor_grid_point_coordinates[axis] = std::max(
        length - 2, 0); // length-2 because that's the left side of the (length-2, length-1) edge.
  } else if (target[axis] < axis_values[0]) {
    is_inbounds[axis] = Bounds::out_of_bounds;
    floor_grid_point_coordinates[axis] = 0;
  } else if (target[axis] > axis_values.back()) {
    is_inbounds[axis] = Bounds::out_of_bounds;
    floor_grid_point_coordinates[axis] = std::max(
        length - 2, 0); // length-2 because that's the left side of the (length-2, length-1) edge.
  } else if (target[axis] == axis_values.back()) {
    is_inbounds[axis] = Bounds::in_bounds;
    floor_grid_point_coordinates[axis] = std::max(
        length - 2, 0); // length-2 because that's the left side of the (length-2, length-1) edge.
  } else {
    is_inbounds[axis] = Bounds::in_bounds;
    std::vector<double>::const_iterator upper =
        std::upper_bound(axis_values.begin(), axis_values.end(), target[axis]);
    floor_grid_point_coordinates[axis] = upper - axis_values.begin() - 1;
  }
}

void RegularGridInterpolatorPrivate::calculate_floor_to_ceiling_fractions() {
  for (std::size_t dim = 0; dim < number_of_axes; ++dim) {
    if (axis_lengths[dim] > 1) {
      floor_to_ceiling_fractions[dim] =
          compute_fraction(target[dim], get_axis_values(dim)[floor_grid_point_coordinates[dim]],
                           get_axis_values(dim)[floor_grid_point_coordinates[dim] + 1]);
    } else {
      floor_to_ceiling_fractions[dim] = 1.0;
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
    for (std::size_t dim = 0; dim < number_of_axes; dim++) {
      if (is_inbounds[dim] == Bounds::out_of_bounds) {
        methods[dim] = extrapolation_methods[dim];
      } else if (is_inbounds[dim] == Bounds::outlaw) {
        // TODO showMessage(MsgLevel::MSG_WARN, stringify("The target is outside the extrapolation
        // limits in dimension ", dim,
        //                                ". Will perform constant extrapolation."));
        methods[dim] = Method::constant;
      }
    }
  }
  reset_hypercube |= !std::equal(previous_methods.begin(), previous_methods.end(), methods.begin());
  if (reset_hypercube) {
    set_hypercube(methods);
  }
}

void RegularGridInterpolatorPrivate::clear_target() {
  target_is_set = false;
  target = std::vector<double>(number_of_axes, 0.);
  results = std::vector<double>(number_of_axes, 0.);
}

void RegularGridInterpolatorPrivate::set_hypercube(std::vector<Method> m_methods) {
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
  for (std::size_t dim = 0; dim < number_of_axes; dim++) {
    if (target_is_set && floor_to_ceiling_fractions[dim] == 0.0) {
      options[dim] = {0};
      reset_hypercube = true;
    } else if (m_methods[dim] == Method::cubic) {
      options[dim] = {-1, 0, 1, 2};
    }
    hypercube_size_hash += options[dim].size() * digit;
    digit *= 10;
  }
  hypercube = {{}};
  for (const auto &list : options) {
    std::vector<std::vector<short>> r;
    for (const auto &x : hypercube) {
      for (const auto item : list) {
        r.push_back(x);
        r.back().push_back(static_cast<short>(item));
      }
    }
    hypercube = std::move(r);
  }
  if (hypercube.size() != previous_size) {
    hypercube_values.resize(hypercube.size(), std::vector<double>(number_of_tables));
    hypercube_weights.resize(hypercube.size());
  }
}

void RegularGridInterpolatorPrivate::calculate_interpolation_coefficients() {
  static constexpr std::size_t floor = 0;
  static constexpr std::size_t ceiling = 1;
  for (std::size_t dim = 0; dim < number_of_axes; dim++) {
    double mu = floor_to_ceiling_fractions[dim];
    if (methods[dim] == Method::cubic) {
      interpolation_coefficients[dim][floor] = 2 * mu * mu * mu - 3 * mu * mu + 1;
      interpolation_coefficients[dim][ceiling] = -2 * mu * mu * mu + 3 * mu * mu;
      cubic_slope_coefficients[dim][floor] =
          (mu * mu * mu - 2 * mu * mu + mu) *
          get_axis_cubic_spacing_ratios(dim, floor)[floor_grid_point_coordinates[dim]];
      cubic_slope_coefficients[dim][ceiling] =
          (mu * mu * mu - mu * mu) *
          get_axis_cubic_spacing_ratios(dim, ceiling)[floor_grid_point_coordinates[dim]];
    } else {
      if (methods[dim] == Method::constant) {
        mu = mu < 0 ? 0 : 1;
      }
      interpolation_coefficients[dim][floor] = 1 - mu;
      interpolation_coefficients[dim][ceiling] = mu;
      cubic_slope_coefficients[dim][floor] = 0.0;
      cubic_slope_coefficients[dim][ceiling] = 0.0;
    }
    weighting_factors[dim][0] = -cubic_slope_coefficients[dim][floor]; // point below floor (-1)
    weighting_factors[dim][1] = interpolation_coefficients[dim][floor] -
                                cubic_slope_coefficients[dim][ceiling]; // floor (0)
    weighting_factors[dim][2] = interpolation_coefficients[dim][ceiling] +
                                cubic_slope_coefficients[dim][floor];   // ceiling (1)
    weighting_factors[dim][3] = cubic_slope_coefficients[dim][ceiling]; // point above ceiling (2)
  }
}

void RegularGridInterpolatorPrivate::set_hypercube_values() {
  if (results.size() != number_of_tables) {
    results.resize(number_of_tables);
    hypercube_values.resize(hypercube.size(), std::vector<double>(number_of_tables));
    hypercube_cache.clear();
  }
  if (hypercube_cache.count({floor_grid_point_index, hypercube_size_hash})) {
    hypercube_values = hypercube_cache.at({floor_grid_point_index, hypercube_size_hash});
    return;
  }
  std::size_t hypercube_index = 0;
  for (const auto &v : hypercube) {
    hypercube_values[hypercube_index] =
        get_grid_point_values_relative(floor_grid_point_coordinates, v);
    ++hypercube_index;
  }
  hypercube_cache[{floor_grid_point_index, hypercube_size_hash}] = hypercube_values;
}

double
RegularGridInterpolatorPrivate::get_grid_point_weighting_factor(const std::vector<short> &v) {
  double weighting_factor = 1.0;
  for (std::size_t dim = 0; dim < number_of_axes; dim++) {
    weighting_factor *= weighting_factors[dim][v[dim] + 1];
  }
  return weighting_factor;
}

std::vector<double> RegularGridInterpolatorPrivate::get_target() const {
  if (!target_is_set) {
    logger->warning(fmt::format("The current target was requested, but no target has been set."));
  }
  return target;
}

// Internal getter methods
std::vector<std::vector<short>> &RegularGridInterpolatorPrivate::get_hypercube() {
  consolidate_methods();
  return hypercube;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_extrapolation_methods() const {
  std::vector<Method> extrap_methods(number_of_axes);
  for (std::size_t dim = 0; dim < number_of_axes; dim++) {
    extrap_methods[dim] = grid_axes[dim].get_extrapolation_method();
  }
  return extrap_methods;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_interpolation_methods() const {
  std::vector<Method> interp_methods(number_of_axes);
  for (std::size_t dim = 0; dim < number_of_axes; dim++) {
    interp_methods[dim] = grid_axes[dim].get_interpolation_method();
  }
  return interp_methods;
}

const std::vector<double> &RegularGridInterpolatorPrivate::get_axis_cubic_spacing_ratios(
    const std::size_t dimension, const std::size_t floor_or_ceiling) const {
  return grid_axes[dimension].get_cubic_spacing_ratios(floor_or_ceiling);
}

} // namespace Btwxt
