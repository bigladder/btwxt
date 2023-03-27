/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

#include <memory>

#include "gtest/gtest.h"

// btwxt
#include "../src/regular-grid-interpolator-private.h"
#include <btwxt/btwxt.h>

#include <fmt/format.h>

#define EXPECT_STDOUT(action, expected_stdout)                                                     \
    {                                                                                              \
        std::stringstream buffer;                                                                  \
        std::streambuf* sbuf = std::cout.rdbuf();                                                  \
        std::cout.rdbuf(buffer.rdbuf());                                                           \
        action std::string capture = buffer.str();                                                 \
        std::cout.rdbuf(sbuf);                                                                     \
        EXPECT_STREQ(expected_stdout.c_str(), buffer.str().c_str());                               \
    }

namespace Btwxt {

template <typename T>
std::vector<std::vector<T>> cartesian_product(const std::vector<std::vector<T>>& v)
{
    std::vector<std::vector<T>> combinations = {{}};
    for (const auto& list : v) {
        std::vector<std::vector<T>> r;
        for (const auto& x : combinations) {
            for (const auto item : list) {
                r.push_back(x);
                r.back().push_back(item);
            }
        }
        combinations = std::move(r);
    }
    return combinations;
}

// return an evenly spaced 1-d vector of doubles.
std::vector<double> linspace(double start, double stop, std::size_t number_of_points);

class GridFixture : public testing::Test {
  public:
    std::vector<std::vector<double>> grid;
    std::vector<std::vector<double>> data_sets;
    std::vector<double> target;
    RegularGridInterpolator interpolator;

    GridFixture() = default;

    virtual void setup() { interpolator = RegularGridInterpolator(grid, data_sets); }
};

class GridFixture2D : public GridFixture {
  protected:
    GridFixture2D()
    {
        grid = {{0, 10, 15}, {4, 6}};
        //         4  6
        data_sets = {{6,
                      3, // 0
                      2,
                      8, // 10
                      4,
                      2}, // 15
                     {12,
                      6, // 0
                      4,
                      16, // 10
                      8,
                      4}}; // 15
        target = {12, 5};
        setup();
        interpolator.set_axis_extrapolation_method(0, Method::linear);
    }
};

class FunctionFixture : public GridFixture {
  public:
    std::vector<std::function<double(std::vector<double>)>> functions;

  protected:
    FunctionFixture() = default;

    void setup() override
    {
        data_sets.resize(functions.size());
        for (std::size_t i = 0u; i < functions.size(); i++) {
            for (auto& grid_point : cartesian_product(grid)) {
                data_sets[i].push_back(functions[i](grid_point));
            }
        }
        interpolator = RegularGridInterpolator(grid, data_sets);
    }
};

class FunctionFixture2D : public FunctionFixture {
  protected:
    FunctionFixture2D()
    {
        grid = {{2.0, 7.0}, {1.0, 2.0, 3.0}};
        functions = {[](std::vector<double> x) -> double { return x[0] * x[1]; }};
        setup();
    }
};

class FunctionFixture4D : public FunctionFixture {
  protected:
    FunctionFixture4D()
    {
        const std::size_t number_of_axes = 4;
        grid.resize(number_of_axes);

        std::size_t axis_len = 10; // could easily change to vector of lengths
        for (std::size_t i = 0; i < number_of_axes; i++) {
            grid[i] = linspace(0.0, 4.5, axis_len);
        }

        functions = {
            [](std::vector<double> x) -> double { return sin(x[0] + x[1]) + cos(x[2] + x[3]); },
            [](std::vector<double> x) -> double { return x[0] + x[1] + x[2] + x[3]; }};

        target = {2.2, 3.3, 1.4, 4.1};
        setup();
    }
};

} // namespace Btwxt