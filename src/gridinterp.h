/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDINTERP_H_
#define GRIDINTERP_H_

// Standard
#include <vector>

namespace Btwxt {

class RegularGridInterpolator{
public:
  RegularGridInterpolator();
  // RegularGridInterpolator(
  //   std::vector< std::vector<double> > grid,
  //   const double* values);
  RegularGridInterpolator(
    std::vector< std::vector<double> > grid,
    std::vector<double> values);

  std::vector< std::vector<double> > grid;
  std::vector<double> values;

  std::size_t get_ndims();
  std::size_t get_nvalues();
  std::vector<size_t> get_dim_lengths();

  double calculate_value_at_target(std::vector<double> target);
  double operator()(std::vector<double> target)
  {
    return calculate_value_at_target(target);
  }
  std::vector<size_t> find_floor(std::vector<double> target);
  double get_value(std::vector<size_t>);

private:
  std::size_t ndims;
  std::size_t nvalues;
  std::vector<size_t> dim_lengths;

  std::vector<double> set_weights(std::vector<double> target);
  double evaluate_linear(std::vector<double> hypercube, std::vector<double> weights);
  size_t grid_floor(double, std::size_t);
  double get_fraction(double, std::size_t);
};

// some free functions
double interpolate(double, double, double);
std::size_t pow(std::size_t, std::size_t);
std::vector< std::vector<std::size_t> > make_binary_list(std::size_t);
std::vector<double> collapse_dimension(std::vector<double>, double);

}
#endif // GRIDINTERP_H_
