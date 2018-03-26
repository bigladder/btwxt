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
  check_target_dimensions(target);
  current_grid_point = GridPoint(target);
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

}
