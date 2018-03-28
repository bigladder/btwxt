/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include<vector>
#include "Eigen/Dense"

//btwxt
#include "btwxt.h"
#include "error.h"

namespace Btwxt {



GridPoint::GridPoint() {};
GridPoint::GridPoint(double* target) {};
GridPoint::GridPoint(const std::vector<double> &target_vector) :
  target(target_vector)
{
  showMessage(MSG_INFO, "GridPoint object constructed from vector!");
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
  the_blob(the_blob)
{
  showMessage(MSG_INFO, "RGI constructed from GriddedData!");
};

RegularGridInterpolator::RegularGridInterpolator(
  const std::vector< std::vector<double> >& grid,
  const std::vector< std::vector<double> >& values
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
  Eigen::ArrayXXd hypercube = collect_hypercube();
  Eigen::ArrayXd eigen_result = evaluate_linear(hypercube);
  std::vector<double> result = eigen_to_vector(eigen_result);
  return result;
};

void RegularGridInterpolator::set_new_grid_point(
  const std::vector<double>& target)
{
  RegularGridInterpolator::check_target_dimensions(target);
  current_grid_point = GridPoint(target);
  the_locator = WhereInTheGridIsThisPoint(current_grid_point, the_blob);
};

std::vector<double> RegularGridInterpolator::get_current_grid_point()
{ return current_grid_point.target; }

void RegularGridInterpolator::clear_current_grid_point() {};

std::size_t RegularGridInterpolator::get_ndims()
{ return the_blob.get_ndims(); };

void RegularGridInterpolator::check_target_dimensions(
  const std::vector<double>& target)
{
  std::size_t ndims = the_blob.get_ndims();
  if (ndims == target.size()) {
    showMessage(MSG_INFO, "Target and GridSpace dimensions match: " + std::to_string(target.size()));
  }
  else {
    std::string message_str = "number of dimensions (" + std::to_string(ndims)
            + ") does not match length of target (" + std::to_string(target.size()) + ").";
    showMessage(MSG_ERR, message_str);
  }
};

std::vector<std::size_t> RegularGridInterpolator::get_current_floor()
{ return the_locator.get_floor(); }

std::vector<double> RegularGridInterpolator::get_current_weights()
{ return the_locator.get_weights(); }





Eigen::ArrayXXd RegularGridInterpolator::collect_hypercube()
{
  // collect all of the points in the interpolation hypercube
  std::size_t ndims = get_ndims();
  std::size_t num_vertices = pow(2, ndims);
  std::vector<std::size_t> point_floor = get_current_floor();
  Eigen::ArrayXXd hypercube(the_blob.get_num_tables(), num_vertices);

  showMessage(MSG_INFO, "we use binary representations to collect hypercube");
  std::vector< std::vector<std::size_t> > vertex_coords = make_binary_list(ndims);

  showMessage(MSG_INFO, "collecting hypercube corners");
  for (std::size_t i=0; i<num_vertices; i++) {
    std::transform(vertex_coords[i].begin( ), vertex_coords[i].end( ),
                   point_floor.begin( ), vertex_coords[i].begin( ),
                   std::plus<std::size_t>());
    hypercube.col(i) = the_blob.get_column(vertex_coords[i]);
  }
  std::cout << hypercube << std::endl;
  return hypercube;
}

Eigen::ArrayXXd RegularGridInterpolator::evaluate_linear(
  Eigen::ArrayXXd hypercube)
{
  // collapse iteratively from n-dim hypercube to a line.
  std::size_t ndims = get_ndims();
  std::vector<double> weights = get_current_weights();

  showMessage(MSG_INFO, "starting interpolation");
  for (std::size_t d=ndims; d>0 ; d--) {
      showMessage(MSG_INFO, "interpolating dim-" + std::to_string(d-1) + ", with frac = " + std::to_string(weights[d-1]));
      hypercube = collapse_dimension(hypercube, weights[d-1]);
  }
  return hypercube;
}

Eigen::ArrayXXd RegularGridInterpolator::collapse_dimension(
  Eigen::ArrayXXd hypercube, const double &frac)
{
  // interpolate along one axis of an n-dimensional hypercube.
  // this flattens a square to a line, or a cube to a square, etc.
  std::size_t half = hypercube.cols()/2;
  Eigen::ArrayXXd output(hypercube.rows(), half);
  for (std::size_t i=0; i<half; i++) {
    output.col(i) = hypercube.col(2*i) * (1-frac) + hypercube.col(2*i+1) * frac;
  }
  std::cout << output << std::endl;
  return output;
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
      std::cout << n << " = "<< binaries[n][0] << ", " << binaries[n][1] << std::endl;
    }
    else if (ndims==3) {
      std::cout << n << " = "<< binaries[n][0] << ", " << binaries[n][1] << ", " << binaries[n][2] << std::endl;
    }
  }
  return binaries;
}

}
