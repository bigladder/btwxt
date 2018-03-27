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


TEST_F(OneDFixture, construct_from_vectors) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 1);
};

TEST_F(TwoDFixture, construct_from_gridded_data) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 2);
};

TEST_F(TwoDFixture, get_values) {
  std::vector<std::size_t> coords = {0, 1};
  std::vector<double> returned_vec = test_gridded_data.get_values(coords);
  ASSERT_THAT(returned_vec, testing::ElementsAre(8, 16));

  coords = {1, 0};
  returned_vec = test_gridded_data.get_values(coords);
  ASSERT_THAT(returned_vec, testing::ElementsAre(3, 6));

  // TODO rebuild get_value() method
  // should return 0 and a warning that we don't have that many tables
  // returned_value = test_gridded_data.get_value(2, coords);
  // EXPECT_EQ(returned_value, 0);

  // should return 0 and a warning that we overran dimension 0
  coords = {7, 0};
  returned_vec = test_gridded_data.get_values(coords);
  ASSERT_THAT(returned_vec, testing::ElementsAre(0));
};

TEST_F(TwoDFixture, return_target) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  std::vector<double> returned_target = test_rgi.get_current_grid_point();
  ASSERT_THAT(returned_target, testing::ElementsAre(12, 5));

  std::vector<std::size_t> point_floor = test_rgi.get_current_floor();
  ASSERT_THAT(point_floor, testing::ElementsAre(1, 0));

  std::vector<double> weights = test_rgi.get_current_weights();
  ASSERT_THAT(weights, testing::ElementsAre(0.4, 0.5));
};

TEST_F(TwoDFixture, interpolate) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  std::vector<double> result = test_rgi.calculate_all_values_at_target();
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(2.9), testing::DoubleEq(5.8)));
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

TEST_F(MismatchedFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();

  // TODO: capture error messages and test that they match expectations
  // we expect two errors that the value table inputs do not match the grid
  // we expect an error that the target dimensions do not match the grid
  test_rgi.set_new_grid_point(target);
};

TEST(CheckSorted, free_check_sorted) {
  std::vector< std::pair< std::vector<double>, bool > > my_vecs =
  {
    {{1,3,5,7,9},  true},
    {{1,3,5,17,9}, false},
    {{9,7,5,3,1},  true},
    {{9,17,5,3,1}, false},
    {{1,3,3,7,9},  false},
    {{9,7,5,5,1},  false},
    {{9},          true}
  };
  bool is_sorted;
  for (auto pair : my_vecs) {
    is_sorted = Btwxt::free_check_sorted(pair.first);
    showMessage(MSG_INFO, (is_sorted? "true" : "false"));
    EXPECT_EQ(is_sorted, pair.second);
  }
};

TEST(FloorFinder, index_below_in_vector) {
  std::vector<double> grid_vector = {1,3,5,7,9};
  double target = 5.3;
  std::size_t expected_floor = 2;
  std::size_t returned_floor = index_below_in_vector(target, grid_vector);
  EXPECT_EQ(returned_floor, expected_floor);
  double expected_weight = 0.15;
  double edge[2] = {grid_vector[returned_floor], grid_vector[returned_floor+1]};
  double returned_weight = compute_fraction(target, edge);
  EXPECT_DOUBLE_EQ(returned_weight, expected_weight);

  // if smaller than all numbers, return vector.size()
  target = 0.3;
  expected_floor = 5;
  returned_floor = index_below_in_vector(target, grid_vector);
  EXPECT_EQ(returned_floor, expected_floor);

  // larger than all numbers returns last index
  target = 9.3;
  expected_floor = 4;
  returned_floor = index_below_in_vector(target, grid_vector);
  EXPECT_EQ(returned_floor, expected_floor);
};
