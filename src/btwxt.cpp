/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include <iostream>
#include <numeric>

//btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {



GridPoint::GridPoint() {};
GridPoint::GridPoint(double* target) {};
GridPoint::GridPoint(const std::vector<double> &target_vector) :
  target(target_vector)
{
  showMessage(MSG_DEBUG, "GridPoint object constructed from vector!");
};




WhereInTheGridIsThisPoint::WhereInTheGridIsThisPoint() {};
WhereInTheGridIsThisPoint::WhereInTheGridIsThisPoint(
  GridPoint& current_grid_point, GriddedData& the_blob
) :
  ndims(the_blob.get_ndims()),
  point_floor(ndims, 0),
  weights(ndims, 0),
  is_inbounds(ndims, false),
  interp_coeffs(ndims, std::vector<double>(2, 0.0)),
  cubic_slope_coeffs(ndims, std::vector<double>(2, 0.0))
{
  find_floor(point_floor, is_inbounds, current_grid_point, the_blob);
  calculate_weights(point_floor, weights, current_grid_point, the_blob);
  consolidate_methods(the_blob.get_interp_methods(),
    the_blob.get_extrap_methods());
  calculate_interp_coeffs();
};

std::vector<std::size_t> WhereInTheGridIsThisPoint::get_floor()
{ return point_floor; }

std::vector<double> WhereInTheGridIsThisPoint::get_weights()
{ return weights; }

std::vector<bool> WhereInTheGridIsThisPoint::get_is_inbounds()
{ return is_inbounds; }

std::vector<int> WhereInTheGridIsThisPoint::get_methods()
{ return methods; }

std::vector< std::vector<double> > WhereInTheGridIsThisPoint::get_interp_coeffs()
{ return interp_coeffs; }

std::vector< std::vector<double> > WhereInTheGridIsThisPoint::get_cubic_slope_coeffs()
{ return cubic_slope_coeffs; }

void WhereInTheGridIsThisPoint::find_floor(
  std::vector<std::size_t> &point_floor, std::vector<bool> &is_inbounds,
  GridPoint& current_grid_point, GriddedData& the_blob)
{
  for (std::size_t d=0; d<ndims; d+=1) {
    std::vector<double> grid_vector = the_blob.get_grid_vector(d);
    point_floor[d] = index_below_in_vector(current_grid_point.target[d], grid_vector);
    if (point_floor[d] == grid_vector.size()) {
      is_inbounds[d] = false;
      point_floor[d] = 0;
    } else if (point_floor[d] == grid_vector.size()-1) {
      is_inbounds[d] = false;
      point_floor[d] -= 1;
    } else {
      is_inbounds[d] = true;
    }
  }
}

void WhereInTheGridIsThisPoint::calculate_weights(
  const std::vector<std::size_t> &point_floor, std::vector<double> &weights,
  GridPoint& current_grid_point, GriddedData& the_blob)
{
  for (std::size_t d=0; d<ndims; d+=1) {
    std::vector<double> grid_vector = the_blob.get_grid_vector(d);
    double edge[] = {grid_vector[point_floor[d]], grid_vector[point_floor[d]+1]};
    weights[d] = compute_fraction(current_grid_point.target[d], edge);
  }
}

void WhereInTheGridIsThisPoint::consolidate_methods(
  const std::vector<int>& interp_methods,
  const std::vector<int>& extrap_methods)
{
  methods = interp_methods;
  for (std::size_t dim=0; dim<ndims; dim++) {
    if (is_inbounds[dim] == false) {
      methods[dim] = extrap_methods[dim];
    }
  }
}

void WhereInTheGridIsThisPoint::calculate_interp_coeffs()
{
  for (std::size_t dim=0; dim<ndims; dim++) {
    double mu = weights[dim];
    if (methods[dim] == CUBIC) {
      interp_coeffs[dim][0] = 2*mu*mu*mu - 3*mu*mu + 1;
      interp_coeffs[dim][1] = -2*mu*mu*mu + 3*mu*mu;
      cubic_slope_coeffs[dim][0] = mu*mu*mu - 2*mu*mu + mu;
      cubic_slope_coeffs[dim][1] = mu*mu*mu - mu*mu;
    } else if (methods[dim] == CONSTANT) {
      mu = mu < 0? 0 : 1;
      interp_coeffs[dim][0] = 1-mu;
      interp_coeffs[dim][1] = mu;
    } else {  // LINEAR
      interp_coeffs[dim][0] = 1-mu;
      interp_coeffs[dim][1] = mu;
    }
  }
}


RegularGridInterpolator::RegularGridInterpolator() {};

RegularGridInterpolator::RegularGridInterpolator(GriddedData &the_blob) :
  the_blob(the_blob),
  current_grid_point(),  // instantiates an empty GridPoint
  cgp_exists(false)
{
  std::size_t ndims{get_ndims()};
  origin_hypercube = make_core_hypercube(ndims);
  showMessage(MSG_DEBUG, "RGI constructed from GriddedData!");
};

RegularGridInterpolator::RegularGridInterpolator(
  const std::vector< std::vector<double> >& grid,
  const std::vector< std::vector<double> >& values
) :
the_blob(grid, values),
current_grid_point(),  // instantiates an empty GridPoint
cgp_exists(false)
{
  std::size_t ndims{get_ndims()};
  origin_hypercube = make_core_hypercube(ndims);
  showMessage(MSG_DEBUG, "RGI constructed from vectors!");
};

double RegularGridInterpolator::calculate_value_at_target(
  std::vector<double> target, std::size_t table_index)
  {
    set_new_grid_point(target);
    std::vector<double> result = interpolation_wrapper();
    return result[table_index];
  };

double RegularGridInterpolator::calculate_value_at_target(
  std::size_t table_index)
  {
    if (cgp_exists) {
      std::vector<double> result = interpolation_wrapper();
      return result[table_index];
    }
    showMessage(MSG_WARN, "No target has been defined!");
    return 0;
  };

std::vector<double> RegularGridInterpolator::calculate_all_values_at_target(
  std::vector<double> target)
{
  set_new_grid_point(target);
  std::vector<double> result = interpolation_wrapper();
  return result;
};

std::vector<double> RegularGridInterpolator::calculate_all_values_at_target()
{
  if (cgp_exists) {
    std::vector<double> result = interpolation_wrapper();
    return result;
  }
  showMessage(MSG_WARN, "No target has been defined!");
  return {0};
};

void RegularGridInterpolator::set_new_grid_point(
  const std::vector<double>& target)
{
  RegularGridInterpolator::check_target_dimensions(target);
  current_grid_point = GridPoint(target);
  cgp_exists = true;
  the_locator = WhereInTheGridIsThisPoint(current_grid_point, the_blob);
};

std::vector<double> RegularGridInterpolator::get_current_grid_point()
{
  if (cgp_exists) {
    return current_grid_point.target;
  }
  showMessage(MSG_WARN, "No target has been defined!");
  return {0};
}

void RegularGridInterpolator::clear_current_grid_point()
{
  current_grid_point = GridPoint();
  cgp_exists = false;
  the_locator = WhereInTheGridIsThisPoint();
};

std::size_t RegularGridInterpolator::get_ndims()
{ return the_blob.get_ndims(); };

void RegularGridInterpolator::check_target_dimensions(
  const std::vector<double>& target)
{
  std::size_t ndims = the_blob.get_ndims();
  if (ndims == target.size()) {
    showMessage(MSG_DEBUG, stringify(
      "Target and GridSpace dimensions match: ", target.size()));
  }
  else {
    showMessage(MSG_ERR, stringify(
      "number of dimensions (", ndims,
      ") does not match length of target (", target.size(), ")."));
  }
};

std::vector<std::size_t> RegularGridInterpolator::get_current_floor()
{
  if (cgp_exists) {
    return the_locator.get_floor();
  }
  showMessage(MSG_WARN, "No target has been defined!");
  return {0};
}

std::vector<double> RegularGridInterpolator::get_current_weights()
{
  if (cgp_exists) {
    return the_locator.get_weights();
  }
  showMessage(MSG_WARN, "No target has been defined!");
  return {0};
}

std::vector< std::vector<double> > RegularGridInterpolator::get_interp_coeffs() {
  return the_locator.get_interp_coeffs();
}

std::vector< std::vector<double> > RegularGridInterpolator::get_cubic_slope_coeffs() {
  return the_locator.get_cubic_slope_coeffs();
}


std::vector<double> RegularGridInterpolator::interpolation_wrapper()
{
  std::vector<double> results = dot_calculator();
  return results;
}

std::vector<double> RegularGridInterpolator::dot_calculator()
{
  // collect all of the points in the interpolation hypercube
  std::size_t ndims = get_ndims();
  std::size_t num_vertices = origin_hypercube.size();
  std::vector<std::size_t> point_floor = get_current_floor();
  std::vector<int> methods = the_locator.get_methods();
  std::vector< std::vector<double> > interp_coeffs = get_interp_coeffs();

  Eigen::ArrayXd result = Eigen::ArrayXd::Zero(the_blob.get_num_tables());

  showMessage(MSG_INFO, "collecting hypercube corners");
  std::vector<std::size_t> temp(ndims);
  for (std::size_t i=0; i<num_vertices; i++) {
    double this_weight = 1.0;
    for (std::size_t dim=0; dim<ndims; dim++) {
        this_weight *= interp_coeffs[dim][origin_hypercube[i][dim]];
    }

    // shift hypercube vertices to point_floor
    std::transform(origin_hypercube[i].begin( ), origin_hypercube[i].end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<int>());

    // add this vertex*weight to the accumulating result
    result += the_blob.get_column(temp) * this_weight;
  }

  // for each cubic-interp axis, get second-order contribution from the rectangle
  for (std::size_t dim=0; dim<ndims; dim++) {
    if (methods[dim] == CUBIC) {
      result += cubic_slope_weighting(dim);
    }
  }

  // TODO for each point in full_hypercube, add third-order contribution.
  if ( std::find(methods.begin(), methods.end(), CUBIC) != methods.end() ) {
    std::vector< std::vector<int> > full_hypercube =
        make_full_hypercube(ndims, the_locator.get_methods());
  }

  showMessage(MSG_DEBUG, stringify("results\n", result));
  return eigen_to_vector(result);
}

Eigen::ArrayXd RegularGridInterpolator::cubic_slope_weighting(const std::size_t this_dim)
{
  Eigen::ArrayXd this_axis_slope_adder = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
  Eigen::ArrayXXd slopes = get_slopes(this_dim);
  Eigen::ArrayXd this_vertex( the_blob.get_num_tables() );
  std::size_t num_vertices = origin_hypercube.size();
  std::vector< std::vector<double> > interp_coeffs = get_interp_coeffs();
  std::vector<double> dim_cubic_slope_coeffs = get_cubic_slope_coeffs()[this_dim];

  for (std::size_t i=0; i<num_vertices; i++) {
    double other_axes_multiplier = 1.0;
    for (std::size_t other_dim=0; other_dim<get_ndims(); other_dim++) {
      if (other_dim != this_dim) {
        other_axes_multiplier *= interp_coeffs[other_dim][origin_hypercube[i][other_dim]];
      }
    }
    if (other_axes_multiplier != 0) {
      this_vertex = dim_cubic_slope_coeffs[origin_hypercube[i][this_dim]] * slopes.col(i);
      showMessage(MSG_DEBUG, stringify("point ", i, ": ", other_axes_multiplier,
        "\n", this_vertex));
      this_axis_slope_adder += this_vertex * other_axes_multiplier;
    }
  }
  showMessage(MSG_DEBUG, stringify("slope contribution, axis-", this_dim,
                                   "\n", this_axis_slope_adder));
  return this_axis_slope_adder;
}

Eigen::ArrayXXd RegularGridInterpolator::get_slopes(const std::size_t& this_dim) {
  std::vector<std::size_t> point_floor = get_current_floor();
  std::size_t ndims = get_ndims();
  std::size_t num_vertices = origin_hypercube.size();
  Eigen::ArrayXXd slopes(the_blob.get_num_tables(), num_vertices);

  std::vector<double> grid_vector = the_blob.get_grid_vector(this_dim);
  double central_spacing = grid_vector[point_floor[this_dim] + 1]
                         - grid_vector[point_floor[this_dim]];
  double spacing_multiplier;

  std::vector<std::size_t> temp(ndims);
  int up, down;
  for (std::size_t i=0; i<num_vertices; i++) {
    up = 1;
    down = -1;
    // shift hypercube vertices to point_floor
    std::transform(origin_hypercube[i].begin( ), origin_hypercube[i].end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<int>());

    spacing_multiplier = the_blob.get_axis_spacing_mult(this_dim,
      origin_hypercube[i][this_dim], point_floor[this_dim]);
    if (temp[this_dim] == 0) {
      down = 0;
    } else if (temp[this_dim] == grid_vector.size()-1) {
      up = 0;
    }
    slopes.col(i) = (the_blob.get_column_near(temp, this_dim, up)
        - the_blob.get_column_near(temp, this_dim, down)) * spacing_multiplier;
  }
  return slopes;
}


// free functions
std::size_t index_below_in_vector(double target, std::vector<double> &my_vec)
  // returns the index of the largest value <= the target
  // if target is greater than all values, returns index of final value
  // if target is less than all values, returns index of final value +1
{
  std::vector<double>::iterator upper;
  upper = std::upper_bound(my_vec.begin(), my_vec.end(), target);
  if ((upper - my_vec.begin()) == 0) {
    return my_vec.size(); }
  return upper - my_vec.begin() - 1;
}

double compute_fraction(double x, double edge[2]) {
  // how far along an edge is the target?
  return (x - edge[0]) / (edge[1] - edge[0]);
}

std::size_t pow(const std::size_t& base, const std::size_t& power) {
  // raise base to a power (both must be size_t)
  if (power == 0) {return 1;}
  else {
    std::size_t result = base;
    for (std::size_t i=1; i<power; i++)
    {
        result = result*base;
    }
    return result;
  }
}

std::vector< std::vector<int> > make_core_hypercube(
  const std::size_t& ndims)
{
  std::vector< std::vector<int> > options(ndims, {0,1});
  return cart_product(options);
}

std::vector< std::vector<int> > make_full_hypercube(
  const std::size_t& ndims, const std::vector<int>& fit_degrees)
{
  std::vector< std::vector<int> > options(ndims, {0,1});

  for (std::size_t dim=0; dim<ndims; dim++) {
    if (fit_degrees[dim] == CUBIC) {
      options[dim] = {-1, 0, 1, 2};
    }
  }
  return cart_product(options);
}

std::vector<std::vector<int>> cart_product (
  const std::vector< std::vector<int> >& v)
{
  std::vector< std::vector<int> > combinations = {{}};
  for (const auto& list : v) {
    std::vector< std::vector<int> > r;
    for (const auto& x : combinations) {
      for (const auto item : list) {
        r.push_back(x);
        r.back().push_back(item);
      }
    }
    combinations = std::move(r);
  }
  return combinations;
}

}
