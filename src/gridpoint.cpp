/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include <iostream>
#include <numeric>

//btwxt
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
  for (std::size_t dim=0; dim<ndims; dim+=1) {
    std::vector<double> grid_vector = the_blob.get_grid_vector(dim);
    point_floor[dim] = index_below_in_vector(current_grid_point.target[dim], grid_vector);
    if (point_floor[dim] == grid_vector.size()) {
      is_inbounds[dim] = false;
      point_floor[dim] = 0;
    } else if (point_floor[dim] == grid_vector.size()-1) {
      is_inbounds[dim] = false;
      point_floor[dim] -= 1;
    } else {
      is_inbounds[dim] = true;
    }
  }
}

void WhereInTheGridIsThisPoint::calculate_weights(
  const std::vector<std::size_t> &point_floor, std::vector<double> &weights,
  GridPoint& current_grid_point, GriddedData& the_blob)
{
  for (std::size_t dim=0; dim<ndims; dim+=1) {
    std::vector<double> grid_vector = the_blob.get_grid_vector(dim);
    double edge[] = {grid_vector[point_floor[dim]], grid_vector[point_floor[dim]+1]};
    weights[dim] = compute_fraction(current_grid_point.target[dim], edge);
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

}
