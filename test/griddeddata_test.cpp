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
  std::size_t ndims = test_gridded_data.get_ndims();
  EXPECT_EQ(ndims, 1);
};

TEST_F(TwoDFixture, construct_from_vectors) {
  std::size_t ndims = test_gridded_data.get_ndims();
  EXPECT_EQ(ndims, 2);

  std::size_t num_tables = test_gridded_data.get_num_tables();
  EXPECT_EQ(num_tables, 2);
};

TEST_F(TwoDFixture, get_grid_vector) {
  std::vector<double> returned_vec = test_gridded_data.get_grid_vector(1);
  EXPECT_THAT(returned_vec, testing::ElementsAre(4, 6));
};

TEST_F(TwoDFixture, get_values) {
  std::vector<std::size_t> coords = {0, 1};
  std::vector<double> returned_vec = test_gridded_data.get_values(coords);
  EXPECT_THAT(returned_vec, testing::ElementsAre(8, 16));

  coords = {1, 0};
  returned_vec = test_gridded_data.get_values(coords);
  EXPECT_THAT(returned_vec, testing::ElementsAre(3, 6));

  // TODO rebuild get_value() method
  // should return 0 and a warning that we don't have that many tables
  // returned_value = test_gridded_data.get_value(2, coords);
  // EXPECT_EQ(returned_value, 0);

  // should return 0 and a warning that we overran dimension 0
  coords = {7, 0};
  returned_vec = test_gridded_data.get_values(coords);
  EXPECT_THAT(returned_vec, testing::ElementsAre(0));
};

TEST_F(TwoDFixture, get_column) {
  std::vector<std::size_t> coords = {0, 1};
  Eigen::ArrayXd returned_col = test_gridded_data.get_column(coords);
  std::cout << returned_col << std::endl;
  std::vector<double> returned_vec = eigen_to_vector(returned_col);
  EXPECT_THAT(returned_vec, testing::ElementsAre(8, 16));
}

TEST_F(MismatchedFixture, ndims) {
  std::size_t ndims = test_gridded_data.get_ndims();
  // TODO: capture error messages and test that they match expectations
  // we expect two errors that the value table inputs do not match the grid
  // we expect an error that the target dimensions do not match the grid
};
