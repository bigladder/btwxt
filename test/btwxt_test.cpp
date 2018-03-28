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
#include "fixtures.hpp"


using namespace Btwxt;


TEST_F(TwoDFixture, construct_from_gridded_data) {
  RegularGridInterpolator rgi_from_grid(test_gridded_data);
  std::size_t ndims = rgi_from_grid.get_ndims();
  EXPECT_EQ(ndims, 2);
};

TEST_F(TwoDFixture, target_basics) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  std::vector<double> returned_target = test_rgi.get_current_grid_point();
  ASSERT_THAT(returned_target, testing::ElementsAre(12, 5));

  std::vector<std::size_t> point_floor = test_rgi.get_current_floor();
  ASSERT_THAT(point_floor, testing::ElementsAre(1, 0));

  std::vector<double> weights = test_rgi.get_current_weights();
  ASSERT_THAT(weights, testing::ElementsAre(0.4, 0.5));
};

TEST_F(TwoDFixture, oobounds_target) {
  std::size_t ndims = test_rgi.get_ndims();
  std::vector<double> oobounds_target = {16, 3};
  test_rgi.set_new_grid_point(oobounds_target);

  std::vector<double> returned_target = test_rgi.get_current_grid_point();
  ASSERT_THAT(returned_target, testing::ElementsAre(16, 3));

  std::vector<std::size_t> point_floor = test_rgi.get_current_floor();
  ASSERT_THAT(point_floor, testing::ElementsAre(1, 0));

  std::vector<double> weights = test_rgi.get_current_weights();
  ASSERT_THAT(weights, testing::ElementsAre(1.2, -0.5));
};

TEST_F(TwoDFixture, interpolate) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  // All values, current target
  std::vector<double> result = test_rgi.calculate_all_values_at_target();
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(2.9), testing::DoubleEq(5.8)));
  // Single value, current target
  double d_result = test_rgi.calculate_value_at_target(0);
  EXPECT_DOUBLE_EQ(d_result, 2.9);

  std::vector<double> another_target = {8.1, 4.2};
  // All values, fresh target
  result = test_rgi.calculate_all_values_at_target(another_target);
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(3.689), testing::DoubleEq(7.378)));
  // Single value, fresh target
  d_result = test_rgi.calculate_value_at_target(another_target, 1);
  EXPECT_DOUBLE_EQ(d_result, 7.378);
};

TEST_F(MismatchedFixture, set_target) {
  // TODO: capture error messages and test that they match expectations
  // we expect two errors that the value table inputs do not match the grid
  // we expect an error that the target dimensions do not match the grid
  test_rgi.set_new_grid_point(target);
};
