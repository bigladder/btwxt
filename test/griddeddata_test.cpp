/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

// btwxt
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
    showMessage(MSG_INFO, stringify("returned column:\n", returned_col));
    std::vector<double> returned_vec = eigen_to_vector(returned_col);
    EXPECT_THAT(returned_vec, testing::ElementsAre(8, 16));
}

TEST_F(TwoDFixture, get_column_up) {
    std::vector<std::size_t> coords = {0, 1};
    Eigen::ArrayXd returned_col = test_gridded_data.get_column_up(coords, 0);
    showMessage(MSG_INFO, stringify("returned column:\n", returned_col));
    std::vector<double> returned_vec = eigen_to_vector(returned_col);
    EXPECT_THAT(returned_vec, testing::ElementsAre(4, 8));

    coords = {2, 1};
    returned_col = test_gridded_data.get_column_up(coords, 0);
    showMessage(MSG_INFO, stringify("returned column:\n", returned_col));
    returned_vec = eigen_to_vector(returned_col);
    EXPECT_THAT(returned_vec, testing::ElementsAre(2, 4));
}

TEST_F(TwoDFixture, get_column_down) {
    std::vector<std::size_t> coords = {0, 1};
    Eigen::ArrayXd returned_col = test_gridded_data.get_column_down(coords, 0);
    showMessage(MSG_INFO, stringify("returned column:\n", returned_col));
    std::vector<double> returned_vec = eigen_to_vector(returned_col);
    EXPECT_THAT(returned_vec, testing::ElementsAre(8, 16));

    coords = {2, 1};
    returned_col = test_gridded_data.get_column_down(coords, 0);
    showMessage(MSG_INFO, stringify("returned column:\n", returned_col));
    returned_vec = eigen_to_vector(returned_col);
    EXPECT_THAT(returned_vec, testing::ElementsAre(4, 8));
}

TEST_F(MismatchedFixture, ndims) {
    std::size_t ndims = test_gridded_data.get_ndims();
    // TODO: capture error messages and test that they match expectations
    // we expect two errors that the value table inputs do not match the grid
    // we expect an error that the target dimensions do not match the grid
};

TEST_F(CubicFixture, get_spacing_multipliers) {
    // for cubic dimension 0: {6, 10, 15, 20}, multipliers should be:
    // flavor 0: {4/4, 5/9, 5/10}
    // flavor 1: {4/9, 5/10, 5/5}
    std::vector<std::vector<double> > expected_results{
            {4.0 / 4, 5.0 / 9,  5.0 / 10},
            {4.0 / 9, 5.0 / 10, 5.0 / 5}};
    double result;
    for (std::size_t flavor = 0; flavor < 2; flavor++) {
        for (std::size_t index = 0; index < 3; index++) {
            result = test_gridded_data.get_axis_spacing_mult(0, flavor, index);
            EXPECT_DOUBLE_EQ(result, expected_results[flavor][index]);
        }
    }
}

TEST(GridAxis, calc_spacing_multipliers) {
    std::vector<double> grid_vector{6, 10, 15, 20, 22};
    int extrapolation_method{CONSTANT};
    int interpolation_method{CUBIC};

    GridAxis test_gridaxis(grid_vector, extrapolation_method, interpolation_method);
    std::vector<std::vector<double> > values = test_gridaxis.spacing_multiplier;
    EXPECT_THAT(values[0], testing::ElementsAre(1, 5.0 / 9, 0.5, 2.0 / 7));
    EXPECT_THAT(values[1], testing::ElementsAre(4.0 / 9, 0.5, 5.0 / 7, 1));
}