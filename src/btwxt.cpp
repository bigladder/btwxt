/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include<vector>

//btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {



WhereInTheGridIsThisPoint::WhereInTheGridIsThisPoint() {};



GridPoint::GridPoint() {};
GridPoint::GridPoint(double* target) {};
GridPoint::GridPoint(std::vector<double> &target_vector) :
  target(target_vector)
{
  showMessage(MSG_INFO, "GridPoint object constructed from vector!");
};


void GridPoint::set_floor_and_weights(
  std::vector<std::size_t> pf,
  std::vector<double> w,
  std::vector<bool> ib
)
{
  point_floor = pf;
  weights = w;
  is_inbounds = ib;
  showMessage(MSG_INFO, "GridPoint floor and weights set!");
};

std::vector<std::size_t> GridPoint::get_floor()
{ return point_floor; }

std::vector<double> GridPoint::get_weights()
{ return weights; }


RegularGridInterpolator::RegularGridInterpolator() {};

RegularGridInterpolator::RegularGridInterpolator(GriddedData &the_blob) :
  the_blob(the_blob)
{
  showMessage(MSG_INFO, "RGI constructed from GriddedData!");
};

RegularGridInterpolator::RegularGridInterpolator(
  std::vector< std::vector<double> > grid,
  std::vector< std::vector<double> > values
) :
the_blob(grid, values)
{
  showMessage(MSG_INFO, "RGI constructed from vectors!");
};

double RegularGridInterpolator::calculate_value_at_target(
  std::vector<double> target, std::size_t table_index)
{return 0.0; };
double RegularGridInterpolator::calculate_value_at_target(
  std::size_t table_index)
{return 0.0; };
std::vector<double> RegularGridInterpolator::calculate_all_values_at_target(
  std::vector<double> target)
{
  std::vector<double> result = {0.0};
  return result;
};
std::vector<double> RegularGridInterpolator::calculate_all_values_at_target()
{
  std::vector<double> result = {0.0};
  return result;
};

void RegularGridInterpolator::set_new_grid_point(std::vector<double> target)
{
  RegularGridInterpolator::check_target_dimensions(target);
  current_grid_point = GridPoint(target);
  RegularGridInterpolator::find_floor_and_weights();
};

std::vector<double> RegularGridInterpolator::get_current_grid_point()
{ return current_grid_point.target; }

void RegularGridInterpolator::clear_current_grid_point() {};

std::size_t RegularGridInterpolator::get_ndims()
{ return the_blob.get_ndims(); };

void RegularGridInterpolator::check_target_dimensions(std::vector<double> target)
{
  std::size_t ndims = the_blob.get_ndims();
  if (ndims == target.size()) {
    showMessage(MSG_INFO, "Target and GridAxes dimensions match: " + std::to_string(target.size()));
  }
  else {
    std::string message_str = "number of dimensions (" + std::to_string(ndims)
            + ") does not match length of target (" + std::to_string(target.size()) + ").";
    showMessage(MSG_ERR, message_str);
  }
};

std::vector<std::size_t> RegularGridInterpolator::get_current_floor()
{ return current_grid_point.get_floor(); }

std::vector<double> RegularGridInterpolator::get_current_weights()
{ return current_grid_point.get_weights(); }


void RegularGridInterpolator::find_floor_and_weights()
{
  std::size_t ndims = RegularGridInterpolator::get_ndims();
  std::vector<std::size_t> point_floor(ndims);
  std::vector<double> weights(ndims);
  std::vector<bool> is_inbounds(ndims);

  RegularGridInterpolator::find_floor(point_floor, is_inbounds);
  RegularGridInterpolator::calculate_weights(point_floor, weights);
  current_grid_point.set_floor_and_weights(point_floor, weights, is_inbounds);
}

void RegularGridInterpolator::find_floor(
  std::vector<std::size_t> &point_floor, std::vector<bool> &is_inbounds)
{
  std::size_t ndims = RegularGridInterpolator::get_ndims();
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

void RegularGridInterpolator::calculate_weights(
  std::vector<std::size_t> &point_floor, std::vector<double> &weights)
{
  std::size_t ndims = RegularGridInterpolator::get_ndims();
  for (std::size_t d=0; d<ndims; d+=1) {
    std::vector<double> grid_vector = the_blob.get_grid_vector(d);
    double edge[] = {grid_vector[point_floor[d]], grid_vector[point_floor[d]+1]};
    weights[d] = compute_fraction(current_grid_point.target[d], edge);
  }
}


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

}
