/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

#include <memory>

#include "gtest/gtest.h"

// btwxt
#include "../src/regular-grid-interpolator-private.h"
#include <btwxt/btwxt.h>

#include <fmt/format.h>

namespace Btwxt {

class GridFixturePrivate : public testing::Test {
  public:
    std::vector<std::vector<double>> grid;
    std::vector<std::vector<double>> values;
    std::vector<double> target;
    RegularGridInterpolatorPrivate interpolator;

    GridFixturePrivate() = default;

    void setup()
    {
        auto logger = std::make_shared<BtwxtContextCourierr>();
        interpolator = RegularGridInterpolatorPrivate(construct_axes(grid, logger), values, logger);
    }
};

class GridFixture2DPrivate : public GridFixturePrivate {
  protected:
    GridFixture2DPrivate()
    {
        grid = grid = {{0, 10, 15}, {4, 6}};
        //         4  6
        values = {{6,
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

class CubicFixture : public GridFixturePrivate {
  protected:
    CubicFixture()
    {
        grid = {{6, 10, 15, 20}, {2, 4, 6, 8}};
        values =
            //  2   4   6    8
            {{4,
              3,
              1.5,
              1, // 6
              5,
              4,
              2,
              1, // 10
              8,
              6,
              3,
              2, // 15
              10,
              8,
              4,
              2}, // 20

             {12,
              10,
              4,
              4, // 6
              16,
              12,
              6,
              4, // 10
              20,
              16,
              8,
              4, // 15
              25,
              20,
              10,
              5}}; // 20

        target = {12, 4.5};
        setup();
        interpolator.set_axis_interpolation_method(0, Method::cubic);
    }
};

class EmptyGridFixturePrivate : public testing::Test {
  protected:
  public:
    std::vector<std::vector<double>> grid;
    RegularGridInterpolatorPrivate interpolator;

    EmptyGridFixturePrivate() = default;

    void setup()
    {
        auto logger = std::make_shared<BtwxtContextCourierr>();
        interpolator = RegularGridInterpolatorPrivate(construct_axes(grid, logger), logger);
    }
};

class GridFixture3DPrivate : public GridFixturePrivate {
  protected:
    GridFixture3DPrivate()
    {
        grid = {{-15, 0.2, 105}, {0, 10, 15}, {4, 6}};
        values = {{6, 3, 2, 8, 4, 2, 3, 6, 13, 2, 0, 15, 3, 6, 13, 2, 0, 15}};
        target = {26.9, 12, 5};
        setup();
        interpolator.set_axis_interpolation_method(0, Method::linear);
        interpolator.set_axis_interpolation_method(1, Method::cubic);
        interpolator.set_axis_interpolation_method(2, Method::linear);
    }
};

} // namespace Btwxt