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
      number_of_dimensions(grid.size()),
      dimension_lengths(number_of_dimensions),
      dimension_step_size(number_of_dimensions),
      temporary_coordinates(number_of_dimensions),
      value_set(number_of_tables, 0.),
      target(number_of_dimensions, 0.),
      point_floor(number_of_dimensions, 0),
      weights(number_of_dimensions, 0.),
      is_inbounds(number_of_dimensions),
      methods(number_of_dimensions, Method::undefined),
      weighting_factors(number_of_dimensions, std::vector<double>(4, 0.)),
      results(number_of_tables, 0.),
      interpolation_coefficients(number_of_dimensions, std::vector<double>(2, 0.)),
      cubic_slope_coefficients(number_of_dimensions, std::vector<double>(2, 0.)),
      logger(std::move(logger)) {
  set_dimension_sizes();
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

void RegularGridInterpolatorPrivate::set_dimension_sizes() {
  number_of_values = 1;
  for (std::size_t dim = number_of_dimensions - 1; /* dim >= 0 */ dim < number_of_dimensions;
       --dim) {
    std::size_t length = grid_axes[dim].get_length();
    if (length == 0) {
      throw BtwxtException(fmt::format("Dimension {} has an axis of length zero.", dim), *logger);
    }
    dimension_lengths[dim] = length;
    dimension_step_size[dim] = number_of_values;
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
  value_set.resize(number_of_tables);
  results.resize(number_of_tables);
  return number_of_tables - 1; // Returns index of new table
}

void RegularGridInterpolator::set_axis_extrapolation_method(const std::size_t dimension,
                                                            const Method method) {
  regular_grid_interpolator->set_axis_extrapolation_method(dimension, method);
}

void RegularGridInterpolator::set_axis_interpolation_method(const std::size_t dimension,
                                                            const Method method) {
  regular_grid_interpolator->set_axis_interpolation_method(dimension, method);
}

void RegularGridInterpolator::set_axis_extrapolation_limits(
    const std::size_t dimension, const std::pair<double, double> &extrap_limits) {
  regular_grid_interpolator->set_axis_extrapolation_limits(dimension, extrap_limits);
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

std::size_t RegularGridInterpolator::get_number_of_dimensions() const {
  return regular_grid_interpolator->number_of_dimensions;
}

std::vector<std::size_t> RegularGridInterpolatorPrivate::get_floor() { return point_floor; }

std::size_t RegularGridInterpolator::get_number_of_tables() const {
  return regular_grid_interpolator->number_of_tables;
}

std::pair<double, double>
RegularGridInterpolator::get_axis_extrapolation_limits(const std::size_t dimension) {
  return regular_grid_interpolator->get_extrapolation_limits(dimension);
}

std::pair<double, double>
RegularGridInterpolatorPrivate::get_extrapolation_limits(std::size_t dimension) const {
  return grid_axes[dimension].get_extrapolation_limits();
}

std::vector<Method> RegularGridInterpolatorPrivate::get_methods() { return methods; }

const std::vector<double> &
RegularGridInterpolatorPrivate::get_axis_values(const std::size_t dimension) {
  return grid_axes[dimension].get_values();
}

// Public printing methods
std::string RegularGridInterpolator::write_data() {
  return regular_grid_interpolator->write_data();
}

std::string RegularGridInterpolatorPrivate::write_data() {
  std::vector<std::size_t> indices(number_of_dimensions, 0);
  std::stringstream output("");

  for (std::size_t dim = 0; dim < number_of_dimensions; ++dim) {
    output << "Axis " << dim + 1 << ",";
  }
  for (std::size_t tab = 0; tab < number_of_tables; ++tab) {
    output << "Value " << tab + 1 << ",";
  }
  output << std::endl;
  for (std::size_t index = 0; index < number_of_values; ++index) {
    for (std::size_t dim = 0; dim < number_of_dimensions; ++dim) {
      output << get_axis_values(dim)[indices[dim]] << ",";
    }
    for (std::size_t tab = 0; tab < number_of_tables; ++tab) {
      output << value_tables[tab][index] << ",";
    }
    output << std::endl;
    ++indices[number_of_dimensions - 1];
    for (std::size_t dim = number_of_dimensions - 1; dim > 0; --dim) {
      if (indices[dim] >= dimension_lengths[dim]) {
        ++indices[dim - 1];
        indices[dim] = 0;
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
  if (v.size() != number_of_dimensions) {
    throw BtwxtException(fmt::format("Target and Gridded Data do not have the same dimensions."),
                         *logger);
  }
  if (target_is_set) {
    if ((v == target) && (methods == get_interpolation_methods())) {
      return;
    }
  }
  target = v;
  target_is_set = true;
  set_floor();
  calculate_weights();
  consolidate_methods();
  calculate_interpolation_coefficients();
  set_results();
}

void RegularGridInterpolatorPrivate::set_results() {
  set_hypercube_values();
  std::fill(results.begin(), results.end(), 0.0);
  for (std::size_t i = 0; i < hypercube.size(); ++i) {
    hypercube_weights[i] = get_vertex_weight(hypercube[i]);
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
RegularGridInterpolatorPrivate::get_value_index(const std::vector<std::size_t> &coords) const {
  std::size_t index = 0;
  for (std::size_t dim = 0; dim < number_of_dimensions; ++dim) {
    index += coords[dim] * dimension_step_size[dim];
  }
  return index;
}

std::vector<double> RegularGridInterpolatorPrivate::get_values(const std::size_t index) {
  for (std::size_t i = 0; i < number_of_tables; ++i) {
    value_set[i] = value_tables[i][index];
  }
  return value_set;
}

std::vector<double>
RegularGridInterpolatorPrivate::get_values(const std::vector<std::size_t> &coords) {
  std::size_t index = get_value_index(coords);
  for (std::size_t i = 0; i < number_of_tables; ++i) {
    value_set[i] = value_tables[i][index];
  }
  return value_set;
}

std::size_t
RegularGridInterpolatorPrivate::get_value_index_relative(const std::vector<std::size_t> &coords,
                                                         const std::vector<short> &translation) {
  int new_coord;
  for (std::size_t dim = 0; dim < coords.size(); dim++) {
    new_coord = static_cast<int>(coords[dim]) + translation[dim];
    if (new_coord < 0) {
      temporary_coordinates[dim] = 0u;
    } else if (new_coord >= (int)dimension_lengths[dim]) {
      temporary_coordinates[dim] = dimension_lengths[dim] - 1u;
    } else {
      temporary_coordinates[dim] = new_coord;
    }
  }
  return get_value_index(temporary_coordinates);
}

std::vector<double>
RegularGridInterpolatorPrivate::get_values_relative(const std::vector<std::size_t> &coords,
                                                    const std::vector<short> &translation) {
  return get_values(get_value_index_relative(coords, translation));
}

void RegularGridInterpolatorPrivate::set_floor() {
  for (std::size_t dim = 0; dim < number_of_dimensions; dim += 1) {
    set_dimension_floor(dim);
  }
  floor_index = get_value_index(point_floor);
}

void RegularGridInterpolatorPrivate::set_dimension_floor(const std::size_t dimension) {
  const auto &axis_values = get_axis_values(dimension);
  int length = static_cast<int>(dimension_lengths[dimension]);
  if (target[dimension] < get_extrapolation_limits(dimension).first) {
    is_inbounds[dimension] = Bounds::outlaw;
    point_floor[dimension] = 0u;
  } else if (target[dimension] > get_extrapolation_limits(dimension).second) {
    is_inbounds[dimension] = Bounds::outlaw;
    point_floor[dimension] = std::max(
        length - 2, 0); // length-2 because that's the left side of the (length-2, length-1) edge.
  } else if (target[dimension] < axis_values[0]) {
    is_inbounds[dimension] = Bounds::out_of_bounds;
    point_floor[dimension] = 0;
  } else if (target[dimension] > axis_values.back()) {
    is_inbounds[dimension] = Bounds::out_of_bounds;
    point_floor[dimension] = std::max(
        length - 2, 0); // length-2 because that's the left side of the (length-2, length-1) edge.
  } else if (target[dimension] == axis_values.back()) {
    is_inbounds[dimension] = Bounds::in_bounds;
    point_floor[dimension] = std::max(
        length - 2, 0); // length-2 because that's the left side of the (length-2, length-1) edge.
  } else {
    is_inbounds[dimension] = Bounds::in_bounds;
    std::vector<double>::const_iterator upper =
        std::upper_bound(axis_values.begin(), axis_values.end(), target[dimension]);
    point_floor[dimension] = upper - axis_values.begin() - 1;
  }
}

void RegularGridInterpolatorPrivate::calculate_weights() {
  for (std::size_t dim = 0; dim < number_of_dimensions; ++dim) {
    if (dimension_lengths[dim] > 1) {
      weights[dim] = compute_fraction(target[dim], get_axis_values(dim)[point_floor[dim]],
                                      get_axis_values(dim)[point_floor[dim] + 1]);
    } else {
      weights[dim] = 1.0;
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
    for (std::size_t dim = 0; dim < number_of_dimensions; dim++) {
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
  target = std::vector<double>(number_of_dimensions, 0.);
  results = std::vector<double>(number_of_dimensions, 0.);
}

void RegularGridInterpolatorPrivate::set_hypercube(std::vector<Method> m_methods) {
  if (m_methods.size() != number_of_dimensions) {
    throw BtwxtException(fmt::format("Error setting hypercube. Methods vector does not "
                                     "have the correct number of dimensions."),
                         *logger);
  }
  std::size_t previous_size = hypercube.size();
  std::vector<std::vector<int>> options(number_of_dimensions, {0, 1});
  reset_hypercube = false;

  hypercube_size_hash = 0;
  std::size_t digit = 1;
  for (std::size_t dim = 0; dim < number_of_dimensions; dim++) {
    if (target_is_set && weights[dim] == 0.0) {
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
  for (std::size_t dim = 0; dim < number_of_dimensions; dim++) {
    double mu = weights[dim];
    if (methods[dim] == Method::cubic) {
      interpolation_coefficients[dim][floor] = 2 * mu * mu * mu - 3 * mu * mu + 1;
      interpolation_coefficients[dim][ceiling] = -2 * mu * mu * mu + 3 * mu * mu;
      cubic_slope_coefficients[dim][floor] =
          (mu * mu * mu - 2 * mu * mu + mu) *
          get_axis_spacing_multipliers(dim, floor)[point_floor[dim]];
      cubic_slope_coefficients[dim][ceiling] =
          (mu * mu * mu - mu * mu) * get_axis_spacing_multipliers(dim, ceiling)[point_floor[dim]];
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
  if (hypercube_cache.count({floor_index, hypercube_size_hash})) {
    hypercube_values = hypercube_cache.at({floor_index, hypercube_size_hash});
    return;
  }
  std::size_t hypercube_index = 0;
  for (const auto &v : hypercube) {
    hypercube_values[hypercube_index] = get_values_relative(point_floor, v);
    ++hypercube_index;
  }
  hypercube_cache[{floor_index, hypercube_size_hash}] = hypercube_values;
}

double RegularGridInterpolatorPrivate::get_vertex_weight(const std::vector<short> &v) {
  double weight = 1.0;
  for (std::size_t dim = 0; dim < number_of_dimensions; dim++) {
    weight *= weighting_factors[dim][v[dim] + 1];
  }
  return weight;
}

std::vector<double> RegularGridInterpolatorPrivate::get_target() const {
  if (!target_is_set) {
    logger->warning(fmt::format("The current target was requested, but no target has been set."));
  }
  return target;
}

// Internal getter methods
std::vector<Bounds> RegularGridInterpolatorPrivate::get_is_inbounds() { return is_inbounds; }

std::vector<double> RegularGridInterpolatorPrivate::get_weights() { return weights; }

std::vector<std::vector<short>> &RegularGridInterpolatorPrivate::get_hypercube() {
  consolidate_methods();
  return hypercube;
}

std::vector<std::vector<double>> &RegularGridInterpolatorPrivate::get_interpolation_coefficients() {
  return interpolation_coefficients;
}

std::vector<std::vector<double>> &RegularGridInterpolatorPrivate::get_cubic_slope_coefficients() {
  return cubic_slope_coefficients;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_extrapolation_methods() const {
  std::vector<Method> extrap_methods(number_of_dimensions);
  for (std::size_t dim = 0; dim < number_of_dimensions; dim++) {
    extrap_methods[dim] = grid_axes[dim].get_extrapolation_method();
  }
  return extrap_methods;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_interpolation_methods() const {
  std::vector<Method> interp_methods(number_of_dimensions);
  for (std::size_t dim = 0; dim < number_of_dimensions; dim++) {
    interp_methods[dim] = grid_axes[dim].get_interpolation_method();
  }
  return interp_methods;
}

const std::vector<double> &
RegularGridInterpolatorPrivate::get_axis_spacing_multipliers(const std::size_t dimension,
                                                             const std::size_t flavor) const {
  return grid_axes[dimension].get_spacing_multipliers(flavor);
}

} // namespace Btwxt
