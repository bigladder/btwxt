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
  origin_hypercube = make_binary_list(get_ndims());
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
  origin_hypercube = make_binary_list(get_ndims());
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
  std::size_t num_vertices = pow(2, ndims);
  std::vector<std::size_t> point_floor = get_current_floor();
  std::vector<double> weights = consider_weights();

  Eigen::ArrayXd result = Eigen::ArrayXd::Zero(the_blob.get_num_tables());

  showMessage(MSG_INFO, "collecting hypercube corners");
  std::vector<std::size_t> temp(ndims);
  for (std::size_t i=0; i<num_vertices; i++) {
    double this_weight = linear_vertex_weighting(origin_hypercube[i], weights);

    // shift hypercube vertices to point_floor
    std::transform(origin_hypercube[i].begin( ), origin_hypercube[i].end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<std::size_t>());

    // add this vertex*weight to the accumulating result
    result += the_blob.get_column(temp) * this_weight;
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
  const std::vector<std::size_t>& coords, const std::vector<double>& weights)
{
  std::size_t ndims = get_ndims();
  std::vector<double> temp(ndims, 0);
  std::transform(coords.begin(), coords.end(), weights.begin(), temp.begin(),
    [](std::size_t p, double w) { return (p==1) ? w : 1-w; });
  double this_weight = std::accumulate(temp.begin(), temp.end(), 1.0,
    std::multiplies<double>());
  return this_weight;
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

std::vector< std::vector<std::size_t> > make_binary_list(
  const std::size_t& ndims)
{
  // produces a list of binary representations of numbers up to 2^ndims.
  // e.g., if ndims=2, this function returns {{0,0}, {0,1}, {1,0}, {1,1}}
  // these binary representations are used to collect all of the points in the interpolation hypercube
  std::vector< std::vector<std::size_t> > binaries;
  for (std::size_t n=0; n<pow(2, ndims); n++) {
    std::size_t i;
    std::size_t b;
    std::vector<std::size_t> single;
    for (i = 1 << (ndims-1); i > 0; i = i / 2) {
      (n & i)? b=1: b=0;
      single.push_back(b);
    }
    binaries.push_back(single);
    if (ndims==2) {
      showMessage(MSG_DEBUG, stringify(n, " = ", binaries[n][0], ", ", binaries[n][1]));
    }
    else if (ndims==3) {
      showMessage(MSG_DEBUG, stringify(n, " = ", binaries[n][0], ", ", binaries[n][1], ", ", binaries[n][2]));
    }
  }
  return binaries;
}

}
