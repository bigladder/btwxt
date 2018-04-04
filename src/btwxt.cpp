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
  is_inbounds(ndims, false)
{
  find_floor(point_floor, is_inbounds, current_grid_point, the_blob);
  calculate_weights(point_floor, weights, current_grid_point, the_blob);
};

std::vector<std::size_t> WhereInTheGridIsThisPoint::get_floor()
{ return point_floor; }

std::vector<double> WhereInTheGridIsThisPoint::get_weights()
{ return weights; }

std::vector<bool> WhereInTheGridIsThisPoint::get_is_inbounds()
{ return is_inbounds; }

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




RegularGridInterpolator::RegularGridInterpolator() {};

RegularGridInterpolator::RegularGridInterpolator(GriddedData &the_blob) :
  the_blob(the_blob),
  current_grid_point(),  // instantiates an empty GridPoint
  cgp_exists(false)
{
  std::size_t ndims{get_ndims()};
  // TODO unwind hardcoding all interpolation methods to linear.
  std::vector<int> interpolation_methods(ndims, LIN_INTR);
  origin_hypercube = make_origin_hypercube(ndims, interpolation_methods);
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
  // TODO unwind hardcoding all interpolation methods to linear.
  std::vector<int> interpolation_methods(ndims, LIN_INTR);
  origin_hypercube = make_origin_hypercube(ndims, interpolation_methods);
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
  std::vector<double> weights = consider_weights();
  std::vector<int> interp_methods = the_blob.get_interp_methods();

  Eigen::ArrayXd result = Eigen::ArrayXd::Zero(the_blob.get_num_tables());

  showMessage(MSG_INFO, "collecting hypercube corners");
  std::vector<std::size_t> temp(ndims);
  for (std::size_t i=0; i<num_vertices; i++) {
    double this_weight = general_vertex_weighting(
      origin_hypercube[i], weights, interp_methods);

    // shift hypercube vertices to point_floor
    std::transform(origin_hypercube[i].begin( ), origin_hypercube[i].end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<int>());

    // add this vertex*weight to the accumulating result
    result += the_blob.get_column(temp) * this_weight;
  }

  // for each cubic-interpolation axis, get the contributions from the slopes
  for (std::size_t axis_index=0; axis_index<ndims; axis_index++) {
    if (interp_methods[axis_index] == CUB_INTR) {
      Eigen::ArrayXXd slopes = get_slopes(axis_index);
      Eigen::ArrayXd slope_contribution = cubic_slope_weighting(
        slopes, weights, axis_index);
      showMessage(MSG_DEBUG, stringify("slope contribution, axis-", axis_index, "\n", slope_contribution));
      result += slope_contribution;
    }
  }

  showMessage(MSG_DEBUG, stringify("results\n", result));
  return eigen_to_vector(result);
}

std::vector<double> RegularGridInterpolator::consider_weights() {
  // get modified weights for extrapolation
  std::size_t ndims = get_ndims();
  std::vector<double> weights = get_current_weights();
  int extrapolation_method;
  std::vector<bool> ibv = the_locator.get_is_inbounds();
  for (std::size_t d=0; d<ndims; d++) {
    if (ibv[d] == false) {
      if (the_blob.get_axis_extrap_method(d) == CON_EXTR) {
        weights[d] = (weights[d] < 0? 0 : 1);
      }
    }
  }
  return weights;
}

double RegularGridInterpolator::linear_vertex_weighting(
  const std::vector<int>& coords, const std::vector<double>& weights)
{
  std::size_t ndims = get_ndims();
  std::vector<double> temp(ndims, 0);
  std::transform(coords.begin(), coords.end(), weights.begin(), temp.begin(),
    [](std::size_t p, double w) { return (p==1) ? w : 1-w; });
  double this_weight = std::accumulate(temp.begin(), temp.end(), 1.0,
    std::multiplies<double>());
  return this_weight;
}

double RegularGridInterpolator::general_vertex_weighting(
  const std::vector<int>& coords, const std::vector<double>& weights,
  const std::vector<int>& interp_methods)
{
  double this_weight = 1.0;
  for (std::size_t i=0; i<coords.size(); i++) {
    double w = weights[i];
    if (interp_methods[i] == CUB_INTR) {
      // NOTE these calculations are only valid with uniform grid spacing
      if (coords[i]==0) {
        this_weight *= 2*w*w*w - 3*w*w + 1;
      } else {
        this_weight *= -2*w*w*w + 3*w*w;
      }
    } else {
      this_weight *= (coords[i]==1) ? w : 1-w;
    }
  }
  return this_weight;
}

Eigen::ArrayXd RegularGridInterpolator::cubic_slope_weighting(
  const Eigen::ArrayXXd& slopes, const std::vector<double>& weights,
  const std::size_t axis_index)
{
  Eigen::ArrayXd this_axis_slope_adder = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
  Eigen::ArrayXd this_vertex( the_blob.get_num_tables() );
  std::size_t num_vertices = origin_hypercube.size();
  double w = weights[axis_index];
  if (w==0 | w==1) { return this_axis_slope_adder; }

  for (std::size_t i=0; i<num_vertices; i++) {
    double other_axes_multiplier = 1.0;
    for (std::size_t j=0; j<weights.size(); j++) {
      if (j != axis_index) {
        other_axes_multiplier *= (origin_hypercube[i][j] == 1? weights[j] : 1-weights[j]);
      }
    }
    if (other_axes_multiplier != 0) {
      if (origin_hypercube[i][axis_index] == 0) {
        this_vertex = (w*w*w - 2*w*w + w) * slopes.col(i);
      } else {
        this_vertex = (w*w*w - w*w) * slopes.col(i);
      }
      showMessage(MSG_DEBUG, stringify("point ", i, ": ", other_axes_multiplier,
        "\n", this_vertex));
      this_axis_slope_adder += this_vertex * other_axes_multiplier;
    }
  }
  return this_axis_slope_adder;
}

Eigen::ArrayXXd RegularGridInterpolator::get_slopes(const std::size_t& axis_index) {
  std::vector<std::size_t> point_floor = get_current_floor();
  std::size_t ndims = get_ndims();
  std::size_t num_vertices = origin_hypercube.size();
  Eigen::ArrayXXd slopes(the_blob.get_num_tables(), num_vertices);

  for (std::size_t i=0; i<num_vertices; i++) {
  }
  std::vector<double> grid_vector = the_blob.get_grid_vector(axis_index);
  std::vector<std::size_t> one(ndims, 0);
  std::vector<std::size_t> zero(ndims, 0);
  std::vector<int> temp(ndims);
  std::vector<std::size_t> up(ndims), down(ndims);
  double grid_up, grid_down;
  one[axis_index] = 1;
  for (std::size_t i=0; i<num_vertices; i++) {
    // shift hypercube vertices to point_floor
    std::transform(origin_hypercube[i].begin( ), origin_hypercube[i].end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<int>());

    if (temp[axis_index] == 0) {
      std::transform(temp.begin(), temp.end(), zero.begin(), down.begin(),
                     std::minus<std::size_t>());
      grid_down = grid_vector[temp[axis_index]];
    } else {
      std::transform(temp.begin(), temp.end(), one.begin(), down.begin(),
                     std::minus<std::size_t>());
      grid_down = grid_vector[temp[axis_index]-1];
    }

    if (temp[axis_index] == grid_vector.size()-1) {
      std::transform(temp.begin(), temp.end(), zero.begin(), up.begin(),
                     std::plus<std::size_t>());
      grid_up = grid_vector[temp[axis_index]];
    } else {
      std::transform(temp.begin(), temp.end(), one.begin(), up.begin(),
                    std::plus<std::size_t>());
      grid_up = grid_vector[temp[axis_index]+1];
    }
    slopes.col(i) = (the_blob.get_column(up) - the_blob.get_column(down)) /
            (grid_up - grid_down);
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

std::vector< std::vector<int> > make_origin_hypercube(
  const std::size_t& ndims, const std::vector<int>& fit_degrees)
{
  std::vector< std::vector<int> > options;
  for (std::size_t dim=0; dim<ndims; dim++) {
    if (fit_degrees[dim] == CUB_INTR) {
      std::vector<int> temp = {-1, 0, 1, 2};
      options.push_back(temp);
    } else {
      std::vector<int> temp = {0, 1};
      options.push_back(temp);
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
