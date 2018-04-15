/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef HYPER_H_
#define HYPER_H_

#include<vector>
#include "Eigen/Dense"

// btwxt
#include "griddeddata.h"
#include "gridpoint.h"

namespace Btwxt{


// class Hypercube{
// public:
//   Hypercube(const std::size_t& ndims);
//
//   std::size_t ndims;
//   std::vector< std::vector<int> > vertices;
// };


class CoreHypercube{
public:
  CoreHypercube();
  CoreHypercube(const std::size_t& ndims);

  std::size_t ndims;
  std::vector< std::vector<int> > vertices;

  void collect_things(WhereInTheGridIsThisPoint&);
  Eigen::ArrayXd compute_core(GriddedData&);
  double weigh_one_vertex(const std::vector<int>& v);

  Eigen::ArrayXd compute_slopes_rectangle(
    const std::size_t& this_dim,
    GriddedData& the_blob);
  double weigh_vertex_slope(const std::vector<int>& v,
    const std::size_t& this_dim);
  Eigen::ArrayXXd get_slopes(
    const std::size_t& this_dim,
    GriddedData& the_blob);

  std::vector<std::size_t> point_floor;
  std::vector<int> methods;
  std::vector< std::vector<double> > interp_coeffs;
  std::vector< std::vector<double> > cubic_slope_coeffs;
};

class FullHypercube{
public:
  FullHypercube(const std::size_t& ndims, const std::vector<int>& methods);

  std::size_t ndims;
  std::vector< std::vector<int> > vertices;

  Eigen::ArrayXd third_order_contributions(
    WhereInTheGridIsThisPoint& the_locator,
    GriddedData& the_blob);
  double weigh_one_vertex(
    const std::vector<int>& v, GriddedData& the_blob);

  std::vector<std::size_t> point_floor;
  std::vector<int> methods;
  std::vector< std::vector<double> > cubic_slope_coeffs;
  std::vector< std::vector<double> > interp_coeffs;
};


// free functions
std::vector< std::vector<int> > make_core_hypercube(const std::size_t& ndims);
std::vector< std::vector<int> > make_full_hypercube(
  const std::size_t& ndims, const std::vector<int>& fit_degrees);
std::vector< std::vector<int> > cart_product(
  const std::vector< std::vector<int> >& v);
}
#endif // HYPER_H_
