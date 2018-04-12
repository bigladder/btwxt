/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDINTERP_H_
#define GRIDINTERP_H_

// Standard
#include<vector>
#include "Eigen/Dense"

// btwxt
#include "griddeddata.h"

// We will eventually split this out into two header files,
//     the public API and the private members.


namespace Btwxt{

class GridPoint{
public:
  // target is an array of doubles specifying the point we are interpolating to.
  GridPoint();
  GridPoint(double* target);
  GridPoint(const std::vector<double> &target_vector);

  std::vector<double> target;
};



class WhereInTheGridIsThisPoint{
public:
  WhereInTheGridIsThisPoint();
  WhereInTheGridIsThisPoint(GridPoint&, GriddedData&);

  std::vector<std::size_t> get_floor();
  std::vector<double> get_weights();
  std::vector<bool> get_is_inbounds();
  std::vector<int> get_methods();
  std::vector< std::vector<double> > get_interp_coeffs();
  std::vector< std::vector<double> > get_cubic_slope_coeffs();

private:
  std::size_t ndims;
  std::vector<std::size_t> point_floor;  // index of grid point <= target
  std::vector<double> weights;
  // TODO upgrade is_inbounds to a family of const ints to allow both
  //     1. outside grid but can extrapolate to, and
  //     2. outside allowed extrapolation zone.
  std::vector<bool> is_inbounds;  // for deciding interpolation vs. extrapolation;
  std::vector<int> methods;
  std::vector< std::vector<double> > interp_coeffs;
  std::vector< std::vector<double> > cubic_slope_coeffs;

  void find_floor(
    std::vector<std::size_t>& point_floor, std::vector<bool>& is_inbounds,
    GridPoint&, GriddedData&);
  void calculate_weights(
    const std::vector<std::size_t>& point_floor, std::vector<double>& weights,
    GridPoint&, GriddedData&);
  void consolidate_methods(const std::vector<int>& interp_methods,
    const std::vector<int>& extrap_methods);
  void calculate_interp_coeffs();
};




// this will be the public-facing class.
class RegularGridInterpolator{
public:
  // GridSpace, GridAxis, AllValueTables, ValueTable are instantiated in RGI constructor.
  RegularGridInterpolator();
  RegularGridInterpolator(GriddedData &the_blob);
  RegularGridInterpolator(
    const std::vector< std::vector<double> >& grid,
    const std::vector< std::vector<double> >& values
  );

  // GridPoint gets instantiated inside calculate_value_at_target
  double calculate_value_at_target(std::vector<double> target, std::size_t table_index);
  double operator()(std::vector<double> target, std::size_t table_index)
  {
    return calculate_value_at_target(target, table_index);
  }

  double calculate_value_at_target(std::size_t table_index);
  double operator()(std::size_t table_index)
  {
    return calculate_value_at_target(table_index);
  }

  std::vector<double> calculate_all_values_at_target(std::vector<double> target);
  std::vector<double> operator()(std::vector<double> target)
  {
    return calculate_all_values_at_target(target);
  }

  std::vector<double> calculate_all_values_at_target();
  std::vector<double> operator()()
  {
    return calculate_all_values_at_target();
  }

  void set_new_grid_point(const std::vector<double>& target);
  std::vector<double> get_current_grid_point();
  void clear_current_grid_point();
  std::size_t get_ndims();
  std::vector<std::size_t> get_current_floor();
  std::vector<double> get_current_weights();
  // TODO move slope-related methods to private
  Eigen::ArrayXd cubic_slope_weighting(const std::size_t this_dim);
  Eigen::ArrayXXd get_slopes(const std::size_t& this_dim);
  std::vector< std::vector<double> > get_interp_coeffs();
  std::vector< std::vector<double> > get_cubic_slope_coeffs();


private:
  GriddedData the_blob;
  bool cgp_exists;
  GridPoint current_grid_point;
  WhereInTheGridIsThisPoint the_locator;
  std::vector< std::vector<int> > origin_hypercube;

  void check_target_dimensions(const std::vector<double> &target);
  std::vector<double> interpolation_wrapper();
  std::vector<double> dot_calculator();
  std::vector<double> consider_weights();
  double linear_vertex_weighting(const std::vector<int>& coords,
    const std::vector<double>& weights);
  double general_vertex_weighting(const std::vector<int>& coords,
    const std::vector<double>& weights, const std::vector<int>& interp_methods);
};


// free functions
std::size_t index_below_in_vector(double target, std::vector<double> &my_vec);
double compute_fraction(double x, double edge[2]);
std::size_t pow(const std::size_t& base, const std::size_t& power);
std::vector< std::vector<int> > make_core_hypercube(const std::size_t& ndims);
std::vector< std::vector<int> > make_full_hypercube(
  const std::size_t& ndims, const std::vector<int>& fit_degrees);
std::vector< std::vector<int> > cart_product(
  const std::vector< std::vector<int> >& v);
}
#endif // GRIDINTERP_H_
