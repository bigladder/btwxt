/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

// btwxt
#include <btwxt.h>
#include <griddeddata.h>
#include <error.h>
#include "fixtures.hpp"


using namespace Btwxt;


TEST_F(TwoDFixture, construct_from_gridded_data) {
  Btwxt::LOG_LEVEL = 0;
  RegularGridInterpolator rgi_from_grid(test_gridded_data);
  std::size_t ndims = rgi_from_grid.get_ndims();
  EXPECT_EQ(ndims, 2);
  Btwxt::LOG_LEVEL = 1;
};

TEST_F(TwoDFixture, target_undefined) {
  Btwxt::LOG_LEVEL = 0;
  showMessage(MSG_INFO, "The test fixture does not instantiate a GridPoint.");
  std::vector<double> returned_target = test_rgi.get_current_grid_point();
  EXPECT_THAT(returned_target, testing::ElementsAre(0));
  std::vector<std::size_t> bad_floor = test_rgi.get_current_floor();
  EXPECT_THAT(bad_floor, testing::ElementsAre(0));
  double bad_result = test_rgi.calculate_value_at_target(0);
  EXPECT_EQ(bad_result, 0);

  showMessage(MSG_INFO, "Define the target; make sure it works now.");
  test_rgi.set_new_grid_point(target);
  returned_target = test_rgi.get_current_grid_point();
  EXPECT_THAT(returned_target, testing::ElementsAre(12, 5));

  showMessage(MSG_INFO, "Clear the target; see that it reverts to warnings.");
  test_rgi.clear_current_grid_point();
  returned_target = test_rgi.get_current_grid_point();
  EXPECT_THAT(returned_target, testing::ElementsAre(0));
  bad_floor = test_rgi.get_current_floor();
  EXPECT_THAT(bad_floor, testing::ElementsAre(0));
  bad_result = test_rgi.calculate_value_at_target(0);
  EXPECT_EQ(bad_result, 0);
  Btwxt::LOG_LEVEL = 1;
}

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

TEST_F(CubicFixture, get_slopes) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);
  std::vector<double> weights = test_rgi.get_current_weights();

  Btwxt::LOG_LEVEL = 0;
  auto start = std::chrono::high_resolution_clock::now();
  Eigen::ArrayXXd slopes = test_rgi.get_slopes(0);
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  showMessage(MSG_INFO, stringify("\n", slopes));
  showMessage(MSG_INFO, stringify("Time to construct slopes: ",
           duration.count(), " microseconds"));

  start = std::chrono::high_resolution_clock::now();
  Eigen::ArrayXd this_axis_slope_adder = test_rgi.cubic_slope_weighting(
    slopes, weights, 0);
  stop = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  showMessage(MSG_INFO, stringify("\n", this_axis_slope_adder));
  showMessage(MSG_INFO, stringify("Time to compute slope contributions: ",
           duration.count(), " microseconds"));
  Btwxt::LOG_LEVEL = 1;
}

TEST_F(CubicFixture, interpolate) {
  test_rgi.set_new_grid_point(target);

  Btwxt::LOG_LEVEL = 0;
  auto start = std::chrono::high_resolution_clock::now();
  std::vector<double> result = test_rgi.calculate_all_values_at_target();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  showMessage(MSG_INFO, stringify("cubic interpolation: [", result[0], ", ", result[1], "]"));
  showMessage(MSG_INFO, stringify("Time to do cubic interpolation: ",
           duration.count(), " microseconds"));
  Btwxt::LOG_LEVEL = 1;
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(4.736), testing::DoubleEq(9.472)));
}

TEST_F(TwoDFixture, interpolate) {
  std::size_t ndims = test_rgi.get_ndims();
  Btwxt::LOG_LEVEL = 0;
  test_rgi.set_new_grid_point(target);

  // All values, current target
  std::vector<double> result = test_rgi.calculate_all_values_at_target();
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(2.9), testing::DoubleEq(5.8)));
  Btwxt::LOG_LEVEL = 1;
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

TEST_F(TwoDFixture, extrapolate) {
  // axis1 is designated constant extrapolation
  std::vector<double> const_extr_target = {10, 3};
  Btwxt::LOG_LEVEL = 0;
  std::vector<double> result = test_rgi(const_extr_target);
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(3), testing::DoubleEq(6)));
  Btwxt::LOG_LEVEL = 1;

  // axis0 is designated linear extrapolation
  std::vector<double> lin_extr_target = {18, 5};
  Btwxt::LOG_LEVEL = 0;
  result = test_rgi(lin_extr_target);
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(1.1), testing::DoubleEq(2.2)));
  Btwxt::LOG_LEVEL = 1;

};

TEST_F(MismatchedFixture, set_target) {
  // TODO: capture error messages and test that they match expectations
  // we expect two errors that the value table inputs do not match the grid
  // we expect an error that the target dimensions do not match the grid
  test_rgi.set_new_grid_point(target);
};
