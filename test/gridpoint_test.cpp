/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>

// vendor
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// btwxt
#include "fixtures.hpp"
#include <griddeddata.h>
#include <gridpoint.h>

using namespace Btwxt;

TEST_F(TwoDFixture, grid_point_basics) {
  GridPoint grid_point(test_gridded_data, target);

  std::vector<std::size_t> point_floor = grid_point.get_floor();
  std::vector<std::size_t> expected_floor{1, 0};
  EXPECT_EQ(point_floor, expected_floor);

  std::vector<double> weights = grid_point.get_weights();
  std::vector<double> expected_weights{0.4, 0.5};
  EXPECT_EQ(weights, expected_weights);
}

TEST_F(TwoDFixture, grid_point_out_of_bounds) {
  std::vector<double> oobounds_vector = {16, 3};
  GridPoint grid_point(test_gridded_data, oobounds_vector);

  std::vector<std::size_t> point_floor = grid_point.get_floor();
  std::vector<std::size_t> expected_floor{1, 0};
  EXPECT_EQ(point_floor, expected_floor);

  std::vector<double> weights = grid_point.get_weights();
  std::vector<double> expected_weights{1.2, -0.5};
  EXPECT_EQ(weights, expected_weights);
}

TEST_F(TwoDFixture, grid_point_consolidate_methods) {
  GridPoint grid_point(test_gridded_data, target);

  std::vector<Method> methods = grid_point.get_methods();
  std::vector<Method> expected_methods{Method::LINEAR, Method::LINEAR};
  EXPECT_EQ(methods, expected_methods);

  std::vector<double> oobounds_vector = {12, 3};
  grid_point = GridPoint(test_gridded_data, oobounds_vector);
  methods = grid_point.get_methods();
  expected_methods = {Method::LINEAR, Method::CONSTANT};
  EXPECT_EQ(methods, expected_methods);
}

TEST_F(TwoDFixture, grid_point_interp_coeffs) {
  GridPoint grid_point(test_gridded_data, target);

  std::vector<std::vector<double>> interp_coeffs = grid_point.get_interp_coeffs();
  std::vector<std::vector<double>> cubic_slope_coeffs = grid_point.get_cubic_slope_coeffs();
  std::vector<double> mu = grid_point.get_weights();

  EXPECT_EQ(interp_coeffs[0][1], mu[0]);
  EXPECT_EQ(interp_coeffs[1][0], 1 - mu[1]);

  EXPECT_EQ(cubic_slope_coeffs[0][0], 0);
  EXPECT_EQ(cubic_slope_coeffs[1][1], 0);
}

TEST_F(TwoDSimpleNormalizationFixture, grid_point_scalar_return) {
    GridPoint grid_point(test_gridded_data, target);
    double return_scalar;
    std::vector<double> results;
    double expected_divisor {6.0};
    double expected_value_at_target {1.5};
    std::vector<double> normalize_target {2.0, 3.0};

    grid_point.set_target(normalize_target);
    return_scalar = grid_point.normalize_grid_values_at_target(0, 1.0);
    EXPECT_EQ(return_scalar, expected_divisor);
}

TEST_F(TwoDSimpleNormalizationFixture, grid_point_compound_scalar_return) {
    GridPoint grid_point(test_gridded_data, target);
    double return_scalar;
    std::vector<double> results;
    double expected_divisor {6.0*4.0};
    double expected_value_at_target {1.5};
    std::vector<double> normalize_target {2.0, 3.0};

    grid_point.set_target(normalize_target);
    return_scalar = grid_point.normalize_grid_values_at_target(0, 4.0);
    EXPECT_EQ(return_scalar, expected_divisor);
}

TEST_F(CubicFixture, grid_point_interp_coeffs) {
  GridPoint grid_point(test_gridded_data, target);

  std::vector<std::vector<double>> interp_coeffs = grid_point.get_interp_coeffs();
  std::vector<std::vector<double>> cubic_slope_coeffs = grid_point.get_cubic_slope_coeffs();
  double mu = grid_point.get_weights()[0];
  std::size_t floor = grid_point.get_floor()[0];

  EXPECT_EQ(interp_coeffs[0][0], 2 * mu * mu * mu - 3 * mu * mu + 1);
  EXPECT_EQ(interp_coeffs[0][1], -2 * mu * mu * mu + 3 * mu * mu);

  EXPECT_EQ(cubic_slope_coeffs[0][0], (mu * mu * mu - 2 * mu * mu + mu)*test_gridded_data.get_axis_spacing_mult(0,0,floor));
  EXPECT_EQ(cubic_slope_coeffs[0][1], (mu * mu * mu - mu * mu)*test_gridded_data.get_axis_spacing_mult(0,1,floor));
}
