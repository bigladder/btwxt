/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include <iostream>

// vendor
#include "gtest/gtest.h"
#include "gmock/gmock.h"

// btwxt
#include <btwxt.h>
#include <griddeddata.h>
#include <hypercube.h>
#include <error.h>
#include "fixtures.hpp"

using namespace Btwxt;

TEST(CoreHypercube, constructor) {
    std::size_t ndims = 3;
    CoreHypercube my_hypercube(ndims);
    EXPECT_EQ(my_hypercube.vertices.size(), 8);
}

TEST(FullHypercube, constructor) {
    std::size_t ndims = 3;
    std::vector<int> fit_degrees{LINEAR, CUBIC, LINEAR};
    FullHypercube my_hypercube(ndims, fit_degrees);
    EXPECT_EQ(my_hypercube.vertices.size(), 16);
}

TEST_F(CubicFixture, hypercube_collect_things) {
    std::size_t ndims = 2;
    CoreHypercube my_hypercube(ndims);
    GridPoint current_grid_point(target);
    WhereInTheGridIsThisPoint the_locator(current_grid_point, test_gridded_data);

    my_hypercube.collect_things(the_locator);

    EXPECT_EQ(my_hypercube.point_floor[0], the_locator.get_floor()[0]);
    EXPECT_EQ(my_hypercube.methods[0], CUBIC);
    double mu = the_locator.get_weights()[0];
    EXPECT_EQ(my_hypercube.interp_coeffs[0][0], 2 * mu * mu * mu - 3 * mu * mu + 1);
    EXPECT_EQ(my_hypercube.interp_coeffs[1][0], 1 - the_locator.get_weights()[1]);
}

TEST_F(CubicFixture, hypercube_weigh_one_vertex) {
    std::size_t ndims = 2;
    CoreHypercube my_hypercube(ndims);
    GridPoint current_grid_point(target);
    WhereInTheGridIsThisPoint the_locator(current_grid_point, test_gridded_data);

    my_hypercube.collect_things(the_locator);
    std::vector<int> this_vertex = my_hypercube.vertices[0];
    double result = my_hypercube.weigh_one_vertex(this_vertex);

    double mu = the_locator.get_weights()[0];
    double expected_result = 2 * mu * mu * mu - 3 * mu * mu + 1;  // dim 0: Cubic
    expected_result *= 1 - the_locator.get_weights()[1];  // dim 1: Linear

    EXPECT_DOUBLE_EQ(result, expected_result);
}

TEST_F(CubicFixture, hypercube_get_slopes) {
    std::size_t ndims = 2;
    CoreHypercube my_hypercube(ndims);
    GridPoint current_grid_point(target);
    WhereInTheGridIsThisPoint the_locator(current_grid_point, test_gridded_data);

    my_hypercube.collect_things(the_locator);
    std::vector<double> slopes = my_hypercube.get_slopes(0, test_gridded_data);

    EXPECT_EQ(slopes.size(), pow(2, ndims));
    std::size_t axis_floor = the_locator.get_floor()[0];
    double expected_value = test_gridded_data.get_axis_spacing_mult(0, 0, axis_floor);
    EXPECT_DOUBLE_EQ(slopes[0], expected_value);  // hypervertex [0, 0]
    EXPECT_DOUBLE_EQ(slopes[1], expected_value);  // hypervertex [0, 1]
    expected_value = test_gridded_data.get_axis_spacing_mult(0, 1, axis_floor);
    EXPECT_DOUBLE_EQ(slopes[2], expected_value);  // hypervertex [1, 0]
    EXPECT_DOUBLE_EQ(slopes[3], expected_value);  // hypervertex [1, 1]
}

TEST_F(CubicFixture, hypercube_weigh_vertex_slope) {
    std::size_t ndims = 2;
    CoreHypercube my_hypercube(ndims);
    GridPoint current_grid_point(target);
    WhereInTheGridIsThisPoint the_locator(current_grid_point, test_gridded_data);

    my_hypercube.collect_things(the_locator);
    std::vector<int> this_vertex = my_hypercube.vertices[0];
    double weight = my_hypercube.weigh_vertex_slope(this_vertex, 0);
    std::vector<double> mus = the_locator.get_weights();
    double mu = mus[0];
    double expected_result = mu * mu * mu - 2 * mu * mu + mu;  // dim 0: Cubic
    expected_result *= 1 - mus[1];  // dim 1: Linear
    EXPECT_DOUBLE_EQ(weight, expected_result);

    this_vertex = my_hypercube.vertices[3];
    weight = my_hypercube.weigh_vertex_slope(this_vertex, 0);
    expected_result = mu * mu * mu - mu * mu;  // dim 0: Cubic
    expected_result *= mus[1];  // dim 1: Linear
    EXPECT_DOUBLE_EQ(weight, expected_result);
}
