/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "Eigen/Dense"

// btwxt
#include <btwxt.h>
#include <griddeddata.h>
#include <error.h>



using namespace Btwxt;


// return an evenly spaced 1-d grid of doubles.
std::vector<double> linspace(double first, double last, std::size_t len) {
  std::vector<double> result(len);
  double step = (last-first) / (len - 1);
  double val = first;
  for (std::size_t i=0; i<len; i++, val+=step) {
    result[i] = val;
  }
  return result;
}


// the functions to interpolate.
double fn0 (double x0, double x1, double x2, double x3)
{ return sin(x0 + x1) + cos(x2 + x3); }

double fn1 (double x0, double x1, double x2, double x3)
{ return (x0 + x1 + x2 + x3); }


class LargeFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target;

  LargeFixture(){
    std::size_t ndims = 4;
    std::vector<std::vector<double> > grid(ndims);
    std::size_t num_tables = 2;

    std::size_t axis_len = 10;  // could easily change to vector of lengths
    std::size_t num_values = 1;
    std::vector<std::size_t> dim_lengths;
    for (std::size_t i=0; i<ndims; i++) {
      grid[i] = linspace(0.0, 4.5, axis_len);
      num_values *= grid[i].size();
      dim_lengths.push_back(grid[i].size());
    }

    std::vector<double> values0(num_values);
    std::vector<double> values1(num_values);
    std::size_t value_loc = 0;
    for (std::size_t i0=0; i0<dim_lengths[0]; i0++) {
      for (std::size_t i1=0; i1<dim_lengths[1]; i1++) {
        for (std::size_t i2=0; i2<dim_lengths[2]; i2++) {
          for (std::size_t i3=0; i3<dim_lengths[3]; i3++) {
            values0[value_loc] = fn0(
              grid[0][i0], grid[1][i1], grid[2][i2], grid[3][i3]);
            values1[value_loc] = fn1(
              grid[0][i0], grid[1][i1], grid[2][i2], grid[3][i3]);
            value_loc++;
          }
        }
      }
    }
    std::vector<std::vector<double> > values = {values0, values1};

    target = {2.2, 3.3, 1.4, 4.1};
    test_gridded_data = GriddedData(grid, values);
    test_rgi = RegularGridInterpolator(test_gridded_data);
  }
};

TEST_F(LargeFixture, construct) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  std::vector<double> returned_target = test_rgi.get_current_grid_point();
  EXPECT_THAT(returned_target, testing::ElementsAre(2.2, 3.3, 1.4, 4.1));

  std::vector<std::size_t> point_floor = test_rgi.get_current_floor();
  EXPECT_THAT(point_floor, testing::ElementsAre(4, 6, 2, 8));

  std::vector<double> weights = test_rgi.get_current_weights();
  EXPECT_NEAR(weights[0], 0.4, 0.0001);
  EXPECT_NEAR(weights[3], 0.2, 0.0001);
};

TEST_F(LargeFixture, calculate) {
  test_rgi.set_new_grid_point(target);

  std::vector<double> result = test_rgi.calculate_all_values_at_target();
  // note: fn0(2.2, 3.3, 1.4, 4.1) evaluates to 0.003.
  EXPECT_NEAR(result[0], -0.0109, 0.0001);
  EXPECT_DOUBLE_EQ(result[1], 11);
};
