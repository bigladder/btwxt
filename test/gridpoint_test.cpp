/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>

// vendor
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// btwxt
#include "fixtures.hpp"

using namespace Btwxt;

TEST_F(TwoDFixture, grid_point_basics) {
  test_rgi.set_new_target(target);

  std::vector<std::size_t> point_floor = test_rgi.regular_grid_interpolator->get_floor();
  std::vector<std::size_t> expected_floor{1, 0};
  EXPECT_EQ(point_floor, expected_floor);

  std::vector<double> weights = test_rgi.regular_grid_interpolator->get_weights();
  std::vector<double> expected_weights{0.4, 0.5};
  EXPECT_EQ(weights, expected_weights);
}

TEST_F(TwoDFixture, grid_point_out_of_bounds) {
  std::vector<double> oobounds_vector = {16, 3};
  test_rgi.set_new_target(oobounds_vector);

  std::vector<std::size_t> point_floor = test_rgi.regular_grid_interpolator->get_floor();
  std::vector<std::size_t> expected_floor{1, 0};
  EXPECT_EQ(point_floor, expected_floor);

  std::vector<double> weights = test_rgi.regular_grid_interpolator->get_weights();
  std::vector<double> expected_weights{1.2, -0.5};
  EXPECT_EQ(weights, expected_weights);
}

TEST_F(TwoDFixture, grid_point_consolidate_methods) {
  test_rgi.set_new_target(target);

  std::vector<Method> methods = test_rgi.regular_grid_interpolator->get_methods();
  std::vector<Method> expected_methods{Method::LINEAR, Method::LINEAR};
  EXPECT_EQ(methods, expected_methods);

  std::vector<double> oobounds_vector = {12, 3};
  test_rgi.set_new_target(oobounds_vector);
  methods = test_rgi.regular_grid_interpolator->get_methods();
  expected_methods = {Method::LINEAR, Method::CONSTANT};
  EXPECT_EQ(methods, expected_methods);
}

TEST_F(TwoDFixture, grid_point_interp_coeffs) {
  test_rgi.set_new_target(target);

  std::vector<std::vector<double>> interp_coeffs = test_rgi.regular_grid_interpolator->get_interp_coeffs();
  std::vector<std::vector<double>> cubic_slope_coeffs = test_rgi.regular_grid_interpolator->get_cubic_slope_coeffs();
  std::vector<double> mu = test_rgi.regular_grid_interpolator->get_weights();

  EXPECT_EQ(interp_coeffs[0][1], mu[0]);
  EXPECT_EQ(interp_coeffs[1][0], 1 - mu[1]);

  EXPECT_EQ(cubic_slope_coeffs[0][0], 0);
  EXPECT_EQ(cubic_slope_coeffs[1][1], 0);
}

TEST_F(CubicFixture, grid_point_interp_coeffs) {
  test_rgi.set_new_target(target);

  std::vector<std::vector<double>> interp_coeffs = test_rgi.regular_grid_interpolator->get_interp_coeffs();
  std::vector<std::vector<double>> cubic_slope_coeffs = test_rgi.regular_grid_interpolator->get_cubic_slope_coeffs();
  double mu = test_rgi.regular_grid_interpolator->get_weights()[0];
  std::size_t floor = test_rgi.regular_grid_interpolator->get_floor()[0];

  EXPECT_EQ(interp_coeffs[0][0], 2 * mu * mu * mu - 3 * mu * mu + 1);
  EXPECT_EQ(interp_coeffs[0][1], -2 * mu * mu * mu + 3 * mu * mu);

  EXPECT_EQ(cubic_slope_coeffs[0][0], (mu * mu * mu - 2 * mu * mu + mu) *
                                          test_rgi.regular_grid_interpolator->get_axis_spacing_mult(0, 0, floor));
  EXPECT_EQ(cubic_slope_coeffs[0][1],
            (mu * mu * mu - mu * mu) * test_rgi.regular_grid_interpolator->get_axis_spacing_mult(0, 1, floor));
}
