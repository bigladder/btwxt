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
    std::vector<double> returned_target;
    std::vector<std::size_t> bad_floor;
    double bad_result;
    std::string ExpectedOut = "  WARNING: No target has been defined!\n";

    // The test fixture does not instantiate a GridPoint.
    EXPECT_STDOUT(returned_target = test_rgi.get_current_grid_point();, ExpectedOut);
    EXPECT_THAT(returned_target, testing::ElementsAre(0));

    EXPECT_STDOUT(bad_floor = test_rgi.get_current_floor();, ExpectedOut);
    EXPECT_THAT(bad_floor, testing::ElementsAre(0));

    EXPECT_STDOUT(bad_result = test_rgi.calculate_value_at_target(0);, ExpectedOut);
    EXPECT_EQ(bad_result, 0);

    // Define the target; make sure it works now.
    test_rgi.set_new_grid_point(target);
    std::string EmptyOut = "";
    EXPECT_STDOUT(returned_target = test_rgi.get_current_grid_point();, EmptyOut);
    EXPECT_THAT(returned_target, testing::ElementsAre(12, 5));

    // Clear the target; see that it reverts to warnings.
    test_rgi.clear_current_grid_point();
    EXPECT_STDOUT(returned_target = test_rgi.get_current_grid_point();, ExpectedOut);
    EXPECT_THAT(returned_target, testing::ElementsAre(0));

    EXPECT_STDOUT(bad_floor = test_rgi.get_current_floor();, ExpectedOut);
    EXPECT_THAT(bad_floor, testing::ElementsAre(0));

    EXPECT_STDOUT(bad_result = test_rgi.calculate_value_at_target(0);, ExpectedOut);
    EXPECT_EQ(bad_result, 0);
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
    std::vector<double> oobounds_target = {16, 3};
    test_rgi.set_new_grid_point(oobounds_target);

    std::vector<double> returned_target = test_rgi.get_current_grid_point();
    ASSERT_THAT(returned_target, testing::ElementsAre(16, 3));

    std::vector<std::size_t> point_floor = test_rgi.get_current_floor();
    ASSERT_THAT(point_floor, testing::ElementsAre(1, 0));

    std::vector<double> weights = test_rgi.get_current_weights();
    ASSERT_THAT(weights, testing::ElementsAre(1.2, -0.5));
};

TEST_F(TwoDFixture, outlaw_target) {
    std::pair<double, double> extrap_limits{-2, 17};
    test_gridded_data.set_axis_extrap_limits(0, extrap_limits);
    RegularGridInterpolator this_rgi(test_gridded_data);
    std::vector<double> outlaw_target = {18, 3};
    std::string ExpectedOut = "  WARNING: The target is outside the extrapolation limits"
                              " in dimension 0. Will perform constant extrapolation.\n";
    EXPECT_STDOUT(this_rgi.set_new_grid_point(outlaw_target);, ExpectedOut);
};

TEST_F(CubicFixture, spacing_multiplier) {
    double result;
    result = test_gridded_data.get_axis_spacing_mult(0, 0, 0);
    EXPECT_DOUBLE_EQ(result, 1.0);

    result = test_gridded_data.get_axis_spacing_mult(0, 1, 0);
    EXPECT_DOUBLE_EQ(result, (10 - 6) / (15.0 - 6.0));

    result = test_gridded_data.get_axis_spacing_mult(0, 0, 1);
    EXPECT_DOUBLE_EQ(result, (15 - 10) / (15.0 - 6.0));

    result = test_gridded_data.get_axis_spacing_mult(0, 1, 2);
    EXPECT_DOUBLE_EQ(result, 1.0);

    result = test_gridded_data.get_axis_spacing_mult(1, 0, 0);
    EXPECT_DOUBLE_EQ(result, 0.0);
}

TEST_F(CubicFixture, calculate_interp_coeffs) {
    test_rgi.set_new_grid_point(target);
    std::vector<std::vector<double> > interp_coeffs =
            test_rgi.get_interp_coeffs();
    std::vector<std::vector<double> > cubic_slope_coeffs =
            test_rgi.get_cubic_slope_coeffs();
    double mu = 0.4;

    EXPECT_DOUBLE_EQ(interp_coeffs[0][0], 2 * mu * mu * mu - 3 * mu * mu + 1);
    EXPECT_DOUBLE_EQ(interp_coeffs[0][1], -2 * mu * mu * mu + 3 * mu * mu);
    EXPECT_DOUBLE_EQ(interp_coeffs[1][0], 0.75);
    EXPECT_DOUBLE_EQ(interp_coeffs[1][1], 0.25);

    EXPECT_DOUBLE_EQ(cubic_slope_coeffs[0][0], mu * mu * mu - 2 * mu * mu + mu);
    EXPECT_DOUBLE_EQ(cubic_slope_coeffs[0][1], mu * mu * mu - mu * mu);
    EXPECT_DOUBLE_EQ(cubic_slope_coeffs[1][0], 0);
    EXPECT_DOUBLE_EQ(cubic_slope_coeffs[1][1], 0);
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
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(3.584), testing::DoubleEq(10.304)));
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

TEST_F(TwoDFixture, invalid_inputs) {
    // TODO: capture error messages and test that they match expectations
    // we expect two errors that the value table inputs do not match the grid
    // we expect an error that the target dimensions do not match the grid
    std::vector<double> short_values = {6, 3, 2, 8, 4};
    EXPECT_THROW(test_gridded_data.add_value_table(short_values);,
                 std::invalid_argument);
    std::vector<double> long_values =  {1, 1, 1, 1, 1, 1, 1};
    EXPECT_THROW(test_gridded_data.add_value_table(long_values);,
                 std::invalid_argument);

    std::vector<double> short_target = {1};
    EXPECT_THROW(test_rgi.set_new_grid_point(short_target);,
                 std::invalid_argument);
    std::vector<double> long_target = {1, 2, 3};
    EXPECT_THROW(test_rgi.set_new_grid_point(long_target);,
                 std::invalid_argument);
};

TEST_F(OneDFixture, cubic_interpolate) {
    Btwxt::LOG_LEVEL = 0;
    test_gridded_data.set_axis_interp_method(0, Method::CUBIC);
    test_rgi = RegularGridInterpolator(test_gridded_data);
    double result = test_rgi.calculate_all_values_at_target(target)[0];
    Btwxt::LOG_LEVEL = 1;
    EXPECT_NEAR(result, 4.804398, 0.0001);
}

TEST_F(TwoDFixture, cubic_interpolate) {
    Btwxt::LOG_LEVEL = 0;
    test_gridded_data.set_axis_interp_method(0, Method::CUBIC);
    test_gridded_data.set_axis_interp_method(1, Method::CUBIC);
    test_rgi = RegularGridInterpolator(test_gridded_data);
    test_rgi.set_new_grid_point(target);

    // All values, current target
    std::vector<double> result = test_rgi.calculate_all_values_at_target();
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(2.876), testing::DoubleEq(5.752)));
    Btwxt::LOG_LEVEL = 1;
}
