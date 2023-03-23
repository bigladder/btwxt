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
                                                 std::shared_ptr<Courierr::Courierr> messenger)
    : regular_grid_interpolator(
          std::make_unique<RegularGridInterpolatorPrivate>(grid, values, messenger)) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis> &grid,
                                                 const std::vector<std::vector<double>> &values,
                                                 std::shared_ptr<Courierr::Courierr> messenger)
    : regular_grid_interpolator(
          std::make_unique<RegularGridInterpolatorPrivate>(grid, values, messenger)) {}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<GridAxis> &grid, const std::vector<std::vector<double>> &values,
    std::shared_ptr<Courierr::Courierr> messenger)
    : grid_axes(grid),
      ndims(grid.size()),
      target(ndims, 0.),
      point_floor(ndims, 0),
      weights(ndims, 0.),
      is_inbounds(ndims),
      methods(ndims, Method::UNDEF),
      weighting_factors(ndims, std::vector<double>(4, 0.)),
      interp_coeffs(ndims, std::vector<double>(2, 0.)),
      cubic_slope_coeffs(ndims, std::vector<double>(2, 0.)),
      num_tables(values.size()),
      results(num_tables, 0.),
      value_tables(values),
      value_set(num_tables, 0.),
      dimension_lengths(ndims),
      dimension_step_size(ndims),
      temp_coords(ndims),
      btwxt_logger(messenger) {
  set_dimension_sizes();
}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<std::vector<double>> &grid, const std::vector<std::vector<double>> &values,
    std::shared_ptr<Courierr::Courierr> messenger)
    : RegularGridInterpolatorPrivate(construct_axes(grid, messenger), values, messenger) {}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<std::vector<double>> &grid, std::shared_ptr<Courierr::Courierr> messenger)
    : RegularGridInterpolatorPrivate(construct_axes(grid, messenger), {}, messenger) {}

RegularGridInterpolatorPrivate::RegularGridInterpolatorPrivate(
    const std::vector<GridAxis> &grid, std::shared_ptr<Courierr::Courierr> messenger)
    : RegularGridInterpolatorPrivate(grid, {}, messenger) {}

RegularGridInterpolator::RegularGridInterpolator(const std::vector<GridAxis> &grid,
                                                 std::shared_ptr<Courierr::Courierr> messenger)
    : regular_grid_interpolator(std::make_unique<RegularGridInterpolatorPrivate>(grid, messenger)) {
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source) {
  *this = source;
  this->regular_grid_interpolator =
      std::make_unique<RegularGridInterpolatorPrivate>(*source.regular_grid_interpolator);
}

RegularGridInterpolator::RegularGridInterpolator(const RegularGridInterpolator &source,
                                                 std::shared_ptr<Courierr::Courierr> messenger) {
  *this = source;
  this->regular_grid_interpolator =
      std::make_unique<RegularGridInterpolatorPrivate>(*source.regular_grid_interpolator);
  this->regular_grid_interpolator->btwxt_logger = messenger;
}

RegularGridInterpolator &RegularGridInterpolator::operator=(const RegularGridInterpolator &source) {
  regular_grid_interpolator =
      std::make_unique<RegularGridInterpolatorPrivate>(*(source.regular_grid_interpolator));
  return *this;
}

void RegularGridInterpolatorPrivate::set_dimension_sizes() {
  num_values = 1;
  for (std::size_t dim = ndims - 1; /* dim >= 0 */ dim < ndims; --dim) {
    std::size_t length = grid_axes[dim].get_length();
    if (length == 0) {
      throw BtwxtException(fmt::format("Dimension {} has an axis of length zero.", dim),
                           *btwxt_logger);
    }
    dimension_lengths[dim] = length;
    dimension_step_size[dim] = num_values;
    num_values *= length;
  }
}

// Public manipulation methods

std::size_t RegularGridInterpolator::add_value_table(const std::vector<double> &value_vector) {
  return regular_grid_interpolator->add_value_table(value_vector);
}

std::size_t
RegularGridInterpolatorPrivate::add_value_table(const std::vector<double> &value_vector) {
  if (value_vector.size() != num_values) {
    throw BtwxtException(fmt::format("Input value table does not match the grid size: {} != {}",
                                     value_vector.size(), num_values),
                         *btwxt_logger);
  }
  value_tables.push_back(value_vector);
  num_tables++;
  value_set.resize(num_tables);
  results.resize(num_tables);
  return num_tables - 1; // Returns index of new table
}

void RegularGridInterpolator::set_axis_extrap_method(const std::size_t dim,
                                                     const Method extrapolation_method) {
  regular_grid_interpolator->set_axis_extrap_method(dim, extrapolation_method);
}

void RegularGridInterpolator::set_axis_interp_method(const std::size_t dim,
                                                     const Method interpolation_method) {
  regular_grid_interpolator->set_axis_interp_method(dim, interpolation_method);
}

void RegularGridInterpolator::set_axis_extrap_limits(
    const std::size_t dim, const std::pair<double, double> &extrap_limits) {
  regular_grid_interpolator->set_axis_extrap_limits(dim, extrap_limits);
}

// Public normalization methods
double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const std::vector<double> &target,
                                                           const double scalar) {
  set_new_target(target);
  return normalize_values_at_target(table_index, scalar);
}

double RegularGridInterpolator::normalize_values_at_target(std::size_t table_index,
                                                           const double scalar) {
  return regular_grid_interpolator->normalize_grid_values_at_target(table_index, scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const std::vector<double> &target,
                                                         const double scalar) {
  set_new_target(target);
  normalize_values_at_target(scalar);
}

void RegularGridInterpolator::normalize_values_at_target(const double scalar) {
  return regular_grid_interpolator->normalize_grid_values_at_target(scalar);
}

void RegularGridInterpolatorPrivate::normalize_grid_values_at_target(const double scalar) {
  if (!target_is_set) {
    throw BtwxtException(fmt::format("Cannot normalize grid values. No target has been set."),
                         *btwxt_logger);
  }
  for (std::size_t table_index = 0; table_index < num_tables; ++table_index) {
    normalize_value_table(table_index, results[table_index] * scalar);
  }
  hypercube_cache.clear();
  set_results();
}

double RegularGridInterpolatorPrivate::normalize_grid_values_at_target(std::size_t table_num,
                                                                       const double scalar) {
  if (!target_is_set) {
    throw BtwxtException(fmt::format("Cannot normalize grid values. No target has been set."),
                         *btwxt_logger);
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
    throw BtwxtException("Attempt to normalize values by zero.", *btwxt_logger);
  }
  scalar = 1.0 / scalar;
  std::transform(table.begin(), table.end(), table.begin(),
                 std::bind(std::multiplies<double>(), std::placeholders::_1, scalar));
}

// Public getter methods

std::size_t RegularGridInterpolator::get_ndims() const {
  return regular_grid_interpolator->get_ndims();
}

std::size_t RegularGridInterpolatorPrivate::get_ndims() const { return ndims; }

std::vector<std::size_t> RegularGridInterpolatorPrivate::get_floor() { return point_floor; }

std::size_t RegularGridInterpolator::get_num_tables() const {
  return regular_grid_interpolator->num_tables;
}

std::pair<double, double> RegularGridInterpolator::get_axis_limits(int dim) {
  return regular_grid_interpolator->get_extrap_limits(dim);
}

std::pair<double, double> RegularGridInterpolatorPrivate::get_extrap_limits(int dim) {
  return grid_axes[dim].extrapolation_limits;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_methods() { return methods; }

const std::vector<double> &RegularGridInterpolatorPrivate::get_grid_vector(const std::size_t dim) {
  return grid_axes[dim].grid;
}

// Public printing methods
std::string RegularGridInterpolator::write_data() {
  return regular_grid_interpolator->write_data();
}

std::string RegularGridInterpolatorPrivate::write_data() {
  std::vector<std::size_t> indices(ndims, 0);
  std::stringstream output("");

  for (std::size_t dim = 0; dim < ndims; ++dim) {
    output << "Axis " << dim + 1 << ",";
  }
  for (std::size_t tab = 0; tab < num_tables; ++tab) {
    output << "Value " << tab + 1 << ",";
  }
  output << std::endl;
  for (std::size_t index = 0; index < num_values; ++index) {
    for (std::size_t dim = 0; dim < ndims; ++dim) {
      output << grid_axes[dim].grid[indices[dim]] << ",";
    }
    for (std::size_t tab = 0; tab < num_tables; ++tab) {
      output << value_tables[tab][index] << ",";
    }
    output << std::endl;
    ++indices[ndims - 1];
    for (std::size_t dim = ndims - 1; dim > 0; --dim) {
      if (indices[dim] >= dimension_lengths[dim]) {
        ++indices[dim - 1];
        indices[dim] = 0;
      }
    }
  }
  return output.str();
}

// Public calculation methods
void RegularGridInterpolator::set_new_target(const std::vector<double> &target) {
  regular_grid_interpolator->set_target(target);
}

void RegularGridInterpolatorPrivate::set_target(const std::vector<double> &v) {
  if (v.size() != ndims) {
    throw BtwxtException(fmt::format("Target and Gridded Data do not have the same dimensions."),
                         *btwxt_logger);
  }
  if (target_is_set) {
    if ((v == target) && (methods == get_interp_methods())) {
      return;
    }
  }
  target = v;
  target_is_set = true;
  set_floor();
  calculate_weights();
  consolidate_methods();
  calculate_interp_coeffs();
  set_results();
}

void RegularGridInterpolatorPrivate::set_results() {
  set_hypercube_values();
  std::fill(results.begin(), results.end(), 0.0);
  for (std::size_t i = 0; i < hypercube.size(); ++i) {
    hypercube_weights[i] = get_vertex_weight(hypercube[i]);
    const auto &values = hypercube_values[i];
    for (std::size_t j = 0; j < num_tables; ++j) {
      results[j] += values[j] * hypercube_weights[i];
    }
  }
}

double RegularGridInterpolator::get_value_at_target(const std::vector<double> &target,
                                                    std::size_t table_index) {
  set_new_target(target);
  return get_value_at_target(table_index);
}

double RegularGridInterpolator::get_value_at_target(std::size_t table_index) {
  return regular_grid_interpolator->get_results()[table_index];
}

std::vector<double> RegularGridInterpolatorPrivate::get_results() {
  if (num_tables == 0u) {
    btwxt_logger->warning(
        fmt::format("There are no value tables in the gridded data. No results returned."));
  }
  if (!target_is_set) {
    btwxt_logger->warning(fmt::format("Results were requested, but no target has been set."));
  }
  return results;
}

std::vector<double> RegularGridInterpolatorPrivate::get_results(const std::vector<double> &target) {
  set_target(target);
  return get_results();
}

std::vector<double>
RegularGridInterpolator::get_values_at_target(const std::vector<double> &target) {
  return regular_grid_interpolator->get_results(target);
}

std::vector<double> RegularGridInterpolator::get_values_at_target() {
  return regular_grid_interpolator->get_results();
}

std::vector<double> RegularGridInterpolator::get_current_target() {
  return regular_grid_interpolator->get_current_target();
}

void RegularGridInterpolator::clear_current_target() {
  regular_grid_interpolator->clear_current_target();
}

// Public logging

void RegularGridInterpolator::set_logger(std::shared_ptr<Courierr::Courierr> logger,
                                         bool set_gridaxes) {
  regular_grid_interpolator->set_logger(logger, set_gridaxes);
}

void RegularGridInterpolatorPrivate::set_logger(std::shared_ptr<Courierr::Courierr> logger,
                                                bool set_grid_axes) {
  btwxt_logger = logger;
  if (set_grid_axes) {
    for (auto &axis : grid_axes) {
      axis.set_logger(logger);
    }
  }
}

Courierr::Courierr &RegularGridInterpolator::get_logger() {
  return *(regular_grid_interpolator->btwxt_logger);
}

// Internal calculation methods

std::size_t
RegularGridInterpolatorPrivate::get_value_index(const std::vector<std::size_t> &coords) const {
  std::size_t index = 0;
  for (std::size_t dim = 0; dim < ndims; ++dim) {
    index += coords[dim] * dimension_step_size[dim];
  }
  return index;
}

std::vector<double> RegularGridInterpolatorPrivate::get_values(const std::size_t index) {
  for (std::size_t i = 0; i < num_tables; ++i) {
    value_set[i] = value_tables[i][index];
  }
  return value_set;
}

std::vector<double>
RegularGridInterpolatorPrivate::get_values(const std::vector<std::size_t> &coords) {
  std::size_t index = get_value_index(coords);
  for (std::size_t i = 0; i < num_tables; ++i) {
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
      temp_coords[dim] = 0u;
    } else if (new_coord >= (int)dimension_lengths[dim]) {
      temp_coords[dim] = dimension_lengths[dim] - 1u;
    } else {
      temp_coords[dim] = new_coord;
    }
  }
  return get_value_index(temp_coords);
}

std::vector<double>
RegularGridInterpolatorPrivate::get_values_relative(const std::vector<std::size_t> &coords,
                                                    const std::vector<short> &translation) {
  return get_values(get_value_index_relative(coords, translation));
}

void RegularGridInterpolatorPrivate::set_floor() {
  for (std::size_t dim = 0; dim < ndims; dim += 1) {
    set_dim_floor(dim);
  }
  floor_index = get_value_index(point_floor);
}

void RegularGridInterpolatorPrivate::set_dim_floor(std::size_t dim) {
  const GridAxis &axis = grid_axes[dim];
  std::size_t l = axis.grid.size();
  if (target[dim] < axis.extrapolation_limits.first) {
    is_inbounds[dim] = Bounds::OUTLAW;
    point_floor[dim] = 0u;
  } else if (target[dim] > axis.extrapolation_limits.second) {
    is_inbounds[dim] = Bounds::OUTLAW;
    point_floor[dim] =
        std::max((int)l - 2, 0); // l-2 because that's the left side of the (l-2, l-1) edge.
  } else if (target[dim] < axis.grid[0]) {
    is_inbounds[dim] = Bounds::OUTBOUNDS;
    point_floor[dim] = 0;
  } else if (target[dim] > axis.grid.back()) {
    is_inbounds[dim] = Bounds::OUTBOUNDS;
    point_floor[dim] =
        std::max((int)l - 2, 0); // l-2 because that's the left side of the (l-2, l-1) edge.
  } else if (target[dim] == axis.grid.back()) {
    is_inbounds[dim] = Bounds::INBOUNDS;
    point_floor[dim] =
        std::max((int)l - 2, 0); // l-2 because that's the left side of the (l-2, l-1) edge.
  } else {
    is_inbounds[dim] = Bounds::INBOUNDS;
    std::vector<double>::const_iterator upper =
        std::upper_bound(axis.grid.begin(), axis.grid.end(), target[dim]);
    point_floor[dim] = upper - axis.grid.begin() - 1;
  }
}

void RegularGridInterpolatorPrivate::calculate_weights() {
  for (std::size_t dim = 0; dim < ndims; ++dim) {
    if (grid_axes[dim].grid.size() > 1) {
      weights[dim] = compute_fraction(target[dim], grid_axes[dim].grid[point_floor[dim]],
                                      grid_axes[dim].grid[point_floor[dim] + 1]);
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
  methods = get_interp_methods();
  if (target_is_set) {
    auto extrap_methods = get_extrap_methods();
    for (std::size_t dim = 0; dim < ndims; dim++) {
      if (is_inbounds[dim] == Bounds::OUTBOUNDS) {
        methods[dim] = extrap_methods[dim];
      } else if (is_inbounds[dim] == Bounds::OUTLAW) {
        // showMessage(MsgLevel::MSG_WARN, stringify("The target is outside the extrapolation
        // limits in dimension ", dim,
        //                                ". Will perform constant extrapolation."));
        methods[dim] = Method::CONSTANT;
      }
    }
  }
  reset_hypercube |= !std::equal(previous_methods.begin(), previous_methods.end(), methods.begin());
  if (reset_hypercube) {
    set_hypercube(methods);
  }
}

void RegularGridInterpolatorPrivate::set_hypercube() { set_hypercube(get_interp_methods()); }

void RegularGridInterpolatorPrivate::clear_current_target() {
  target_is_set = false;
  target = std::vector<double>(ndims, 0.);
  results = std::vector<double>(ndims, 0.);
}

void RegularGridInterpolatorPrivate::set_hypercube(std::vector<Method> m_methods) {
  if (m_methods.size() != ndims) {
    throw BtwxtException(fmt::format("Error setting hypercube. Methods vector does not "
                                     "have the correct number of dimensions."),
                         *btwxt_logger);
  }
  std::size_t previous_size = hypercube.size();
  std::vector<std::vector<int>> options(ndims, {0, 1});
  reset_hypercube = false;

  hypercube_size_hash = 0;
  std::size_t digit = 1;
  for (std::size_t dim = 0; dim < ndims; dim++) {
    if (target_is_set && weights[dim] == 0.0) {
      options[dim] = {0};
      reset_hypercube = true;
    } else if (m_methods[dim] == Method::CUBIC) {
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
    hypercube_values.resize(hypercube.size(), std::vector<double>(num_tables));
    hypercube_weights.resize(hypercube.size());
  }
}

void RegularGridInterpolatorPrivate::calculate_interp_coeffs() {
  for (std::size_t dim = 0; dim < ndims; dim++) {
    double mu = weights[dim];
    if (methods[dim] == Method::CUBIC) {
      interp_coeffs[dim][0] = 2 * mu * mu * mu - 3 * mu * mu + 1;
      interp_coeffs[dim][1] = -2 * mu * mu * mu + 3 * mu * mu;
      cubic_slope_coeffs[dim][0] =
          (mu * mu * mu - 2 * mu * mu + mu) * get_axis_spacing_mult(dim, 0, point_floor[dim]);
      cubic_slope_coeffs[dim][1] =
          (mu * mu * mu - mu * mu) * get_axis_spacing_mult(dim, 1, point_floor[dim]);
    } else {
      if (methods[dim] == Method::CONSTANT) {
        mu = mu < 0 ? 0 : 1;
      }
      interp_coeffs[dim][0] = 1 - mu;
      interp_coeffs[dim][1] = mu;
      cubic_slope_coeffs[dim][0] = 0.0;
      cubic_slope_coeffs[dim][1] = 0.0;
    }
    weighting_factors[dim][0] = -cubic_slope_coeffs[dim][0]; // point below floor (-1)
    weighting_factors[dim][1] = interp_coeffs[dim][0] - cubic_slope_coeffs[dim][1]; // floor (0)
    weighting_factors[dim][2] = interp_coeffs[dim][1] + cubic_slope_coeffs[dim][0]; // ceiling (1)
    weighting_factors[dim][3] = cubic_slope_coeffs[dim][1]; // point above ceiling (2)
  }
}

void RegularGridInterpolatorPrivate::set_hypercube_values() {
  if (results.size() != num_tables) {
    results.resize(num_tables);
    hypercube_values.resize(hypercube.size(), std::vector<double>(num_tables));
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
  for (std::size_t dim = 0; dim < ndims; dim++) {
    weight *= weighting_factors[dim][v[dim] + 1];
  }
  return weight;
}

std::vector<double> RegularGridInterpolatorPrivate::get_current_target() {
  if (!target_is_set) {
    btwxt_logger->warning(
        fmt::format("The current target was requested, but no target has been set."));
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

std::vector<std::vector<double>> RegularGridInterpolatorPrivate::get_interp_coeffs() {
  return interp_coeffs;
}

std::vector<std::vector<double>> RegularGridInterpolatorPrivate::get_cubic_slope_coeffs() {
  return cubic_slope_coeffs;
}

std::pair<double, double>
RegularGridInterpolatorPrivate::get_extrap_limits(const std::size_t dim) const {
  return grid_axes[dim].extrapolation_limits;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_extrap_methods() const {
  std::vector<Method> extrap_methods(ndims);
  for (std::size_t dim = 0; dim < ndims; dim++) {
    extrap_methods[dim] = grid_axes[dim].extrapolation_method;
  }
  return extrap_methods;
}

std::vector<Method> RegularGridInterpolatorPrivate::get_interp_methods() const {
  std::vector<Method> interp_methods(ndims);
  for (std::size_t dim = 0; dim < ndims; dim++) {
    interp_methods[dim] = grid_axes[dim].interpolation_method;
  }
  return interp_methods;
}

double RegularGridInterpolatorPrivate::get_axis_spacing_mult(const std::size_t dim,
                                                             const std::size_t flavor,
                                                             const std::size_t index) const {
  if (grid_axes[dim].interpolation_method == Method::CUBIC) {
    return grid_axes[dim].get_spacing_multiplier(flavor, index);
  } else {
    return 0.0;
  }
}

} // namespace Btwxt
