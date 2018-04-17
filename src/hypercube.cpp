/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>

// btwxt
// #include "griddeddata.h"
#include "hypercube.h"
#include "error.h"

namespace Btwxt {

// Hypercube::Hypercube(const std::size_t& ndims) {};

CoreHypercube::CoreHypercube() {};

CoreHypercube::CoreHypercube(const std::size_t& ndims) :
  ndims(ndims)
{
  vertices = make_core_hypercube(ndims);
};

void CoreHypercube::collect_things(WhereInTheGridIsThisPoint& the_locator) {
  point_floor = the_locator.get_floor();
  methods = the_locator.get_methods();
  interp_coeffs = the_locator.get_interp_coeffs();
  if ( std::find(methods.begin(), methods.end(), CUBIC) != methods.end() ) {
    cubic_slope_coeffs = the_locator.get_cubic_slope_coeffs();
  }
}

Eigen::ArrayXd CoreHypercube::compute_core(GriddedData& the_blob)
{
  showMessage(MSG_INFO, "computing core hypercube.");

  Eigen::ArrayXd result = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
  Eigen::ArrayXd values = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
  for (auto v: vertices) {
    values = the_blob.get_column_near(point_floor, v);
    double weight = weigh_one_vertex(v);
    showMessage(MSG_DEBUG, stringify("vertex_weight: ", weight));
    result += values * weight;
  }
  showMessage(MSG_DEBUG, stringify("core contribution = \n", result));
  return result;
}

double CoreHypercube::weigh_one_vertex(const std::vector<int>& v)
{
  double this_weight = 1.0;
  for (std::size_t dim=0; dim<ndims; dim++) {
      this_weight *= interp_coeffs[dim][v[dim]];
  }
  return this_weight;
}


Eigen::ArrayXd CoreHypercube::compute_slopes_rectangle(
  const std::size_t& this_dim,
  GriddedData& the_blob)
{
  std::size_t num_vertices = vertices.size();

  std::vector<double> slopes = get_slopes(this_dim, the_blob);
  Eigen::ArrayXd this_axis_slope_adder = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
  Eigen::ArrayXd this_vertex( the_blob.get_num_tables() );

  std::vector<std::size_t> temp(ndims);
  double weight;
  int up, down;
  std::size_t i{0};
  for (auto v: vertices) {
    up = 1;
    down = -1;
    // shift hypercube vertices to point_floor
    std::transform(v.begin( ), v.end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<int>());
    if (temp[this_dim] == 0) { down = 0; }
    else if (temp[this_dim] == the_blob.dimension_lengths[this_dim]-1) { up = 0; }
    this_vertex = the_blob.get_column_near(temp, this_dim, up)
                - the_blob.get_column_near(temp, this_dim, down);
    weight = weigh_vertex_slope(v, this_dim);
    this_axis_slope_adder += this_vertex * weight * slopes[i];
    i++;
  }
  showMessage(MSG_DEBUG, stringify("this_axis_slope_adder = \n", this_axis_slope_adder));
  return this_axis_slope_adder;
}

double CoreHypercube::weigh_vertex_slope(const std::vector<int>& v,
  const std::size_t& this_dim)
{
  double weight { cubic_slope_coeffs[this_dim][v[this_dim]] };
  for (std::size_t other_dim=0; other_dim<ndims; other_dim++) {
    if (other_dim != this_dim) {
      weight *= interp_coeffs[other_dim][v[other_dim]];
    }
  }
  showMessage(MSG_DEBUG, stringify("vertex slope weight, axis-", this_dim, weight));
  return weight;
}

std::vector<double> CoreHypercube::get_slopes(const std::size_t& this_dim,
  GriddedData& the_blob)
{
  std::vector<double> slopes(vertices.size());
  std::size_t i{0};
  for (auto v: vertices) {
    slopes[i] = the_blob.get_axis_spacing_mult(
      this_dim, v[this_dim], point_floor[this_dim]);
    i++;
  }
  return slopes;
}



FullHypercube::FullHypercube(const std::size_t& ndims,
  const std::vector<int>& methods) :
  ndims(ndims),
  methods(methods),
  vertices(make_full_hypercube(ndims, methods))
{};

Eigen::ArrayXd FullHypercube::third_order_contributions(
  WhereInTheGridIsThisPoint& the_locator,
  GriddedData& the_blob)
{
  point_floor = the_locator.get_floor();
  cubic_slope_coeffs = the_locator.get_cubic_slope_coeffs();
  interp_coeffs = the_locator.get_interp_coeffs();

  Eigen::ArrayXd third_order_total = Eigen::ArrayXd::Zero(the_blob.get_num_tables());
  Eigen::ArrayXd values = Eigen::ArrayXd::Zero(the_blob.get_num_tables());

  for (auto v : vertices) {
    std::vector<std::size_t> temp(ndims);
    std::transform(v.begin( ), v.end( ),
                   point_floor.begin( ), temp.begin( ),
                   std::plus<int>());

    for (std::size_t dim=0; dim<ndims; dim++) {
      if (temp[dim] > the_blob.dimension_lengths[dim]) {
        temp[dim] = 0;  // because temp is unsigned, a negative value will wrap around
      } else if (temp[dim] == the_blob.dimension_lengths[dim]) {
        temp[dim] -= 1;
      }
    }
    values = the_blob.get_column(temp);
    // showMessage(MSG_DEBUG, stringify("full_hypercube vertex:\n", values));
    third_order_total += values * weigh_one_vertex(v, the_blob);
  }
  showMessage(MSG_DEBUG, stringify("third_order_total = \n", third_order_total));
  return third_order_total;
}

double FullHypercube::weigh_one_vertex(
  const std::vector<int>& v, GriddedData& the_blob)
{
  double weight = 1.0;
  std::size_t flavor;
  for (std::size_t dim=0; dim<ndims; dim++) {
    if (methods[dim] == CUBIC) {
      if (v[dim]==-1 | v[dim]==1) {
        flavor = 0;
      } else {flavor = 1; }
      double spacing_multiplier = the_blob.get_axis_spacing_mult(dim,
        flavor, point_floor[dim]);
      weight *= cubic_slope_coeffs[dim][flavor] * spacing_multiplier;
    } else {
      weight *= interp_coeffs[dim][v[dim]];
    }
  }
  // showMessage(MSG_DEBUG, stringify("third order vertex weight:\n", weight));
  return weight;
}



// free functions
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
