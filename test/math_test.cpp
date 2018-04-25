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
#include <error.h>

using namespace Btwxt;


// Unit tests on free functions, generally mathematically oriented.


TEST(GriddedData, free_check_sorted) {
    Btwxt::LOG_LEVEL = 0;
    std::vector<std::pair<std::vector<double>, bool> > my_vecs =
            {
                    {{1, 3,  5, 7,  9}, true},
                    {{1, 3,  5, 17, 9}, false},
                    {{9, 7,  5, 3,  1}, true},
                    {{9, 17, 5, 3,  1}, false},
                    {{1, 3,  3, 7,  9}, false},
                    {{9, 7,  5, 5,  1}, false},
                    {{9},               true}
            };
    bool is_sorted;
    for (auto pair : my_vecs) {
        is_sorted = Btwxt::free_check_sorted(pair.first);
        showMessage(MSG_DEBUG, (is_sorted ? "true" : "false"));
        EXPECT_EQ(is_sorted, pair.second);
    }
    Btwxt::LOG_LEVEL = 1;
}

TEST(GriddedData, locate_coords) {
    std::vector<std::size_t> coords = {2, 3};
    std::vector<std::size_t> dimension_lengths = {5, 7};
    std::size_t index = locate_coords(coords, dimension_lengths);
    EXPECT_EQ(index, 17);

    coords = {2, 3, 2};
    dimension_lengths = {5, 7, 3};
    index = locate_coords(coords, dimension_lengths);
    EXPECT_EQ(index, 87);
}

TEST(GridPoint, locate_in_dim) {
    std::vector<double> grid_vector = {1, 3, 5, 7, 9};
    std::pair<double, double> extrap_limits{0, 11};
    double target = 5.3;
    int is_inbounds;
    std::size_t dim_floor;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, INBOUNDS);
    EXPECT_EQ(dim_floor, 2);

    target = 0.3;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, OUTBOUNDS);
    EXPECT_EQ(dim_floor, 0);

    target = 10.3;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, OUTBOUNDS);
    EXPECT_EQ(dim_floor, 3);

    target = -0.3;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, OUTLAW);
    EXPECT_EQ(dim_floor, 0);

    target = 11.3;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, OUTLAW);
    EXPECT_EQ(dim_floor, 3);

    extrap_limits = {-DBL_MAX, DBL_MAX};
    target = -0.3;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, OUTBOUNDS);
    EXPECT_EQ(dim_floor, 0);

    target = 11.3;
    locate_in_dim(target, is_inbounds, dim_floor, grid_vector, extrap_limits);
    EXPECT_EQ(is_inbounds, OUTBOUNDS);
    EXPECT_EQ(dim_floor, 3);
}

TEST(GridPoint, index_below_in_vector) {
    std::vector<double> grid_vector = {1, 3, 5, 7, 9};
    double target = 5.3;
    std::size_t expected_floor = 2;
    std::size_t returned_floor = index_below_in_vector(target, grid_vector);
    EXPECT_EQ(returned_floor, expected_floor);
    double expected_weight = 0.15;
    double edge[2] = {grid_vector[returned_floor], grid_vector[returned_floor + 1]};
    double returned_weight = compute_fraction(target, edge);
    EXPECT_DOUBLE_EQ(returned_weight, expected_weight);
}

TEST(GridPoint, compute_fraction) {
    double x = 4.3;
    double edge[2] = {4, 6};
    double weight = compute_fraction(x, edge);
    EXPECT_DOUBLE_EQ(weight, 0.15);
}

TEST(Btwxt, pow) {
    std::size_t base = 2;
    std::size_t power = 4;
    std::size_t result = pow(base, power);
    EXPECT_EQ(result, 16);
}

TEST(Hypercube, make_origin_hypercube) {
    std::size_t ndims = 3;
    std::vector<int> fit_degrees = {LINEAR, CUBIC, LINEAR};
    std::vector<std::vector<int>> result = make_full_hypercube(
            ndims, fit_degrees);
    EXPECT_EQ(result.size(), 2 * 4 * 2);
    EXPECT_THAT(result[0], testing::ElementsAre(0, -1, 0));
    EXPECT_THAT(result[2], testing::ElementsAre(0, 0, 0));
    EXPECT_THAT(result[12], testing::ElementsAre(1, 1, 0));
    EXPECT_THAT(result[15], testing::ElementsAre(1, 2, 1));
}

TEST(Hypercube, cart_product) {
    std::vector<std::vector<int> > v = {
            {1, 2, 3},
            {4, 5},
            {6, 7, 8, 9}
    };
    std::vector<std::vector<int>> result = cart_product(v);
    EXPECT_EQ(result.size(), 3 * 2 * 4);
    EXPECT_THAT(result[0], testing::ElementsAre(1, 4, 6));
    EXPECT_THAT(result[1], testing::ElementsAre(1, 4, 7));
    EXPECT_THAT(result[10], testing::ElementsAre(2, 4, 8));
    EXPECT_THAT(result[3 * 2 * 4 - 1], testing::ElementsAre(3, 5, 9));
}
