/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef TEST_FIXTURE_HPP_
#define TEST_FIXTURE_HPP_

#include <memory>

#include "gtest/gtest.h"

// btwxt
#include <btwxt.h>
#include <error.h>
#include <griddeddata.h>

#include <fmt/format.h>

#define EXPECT_STDOUT(action, ExpectedOut)                                                         \
  {                                                                                                \
    std::stringstream buffer;                                                                      \
    std::streambuf *sbuf = std::cout.rdbuf();                                                      \
    std::cout.rdbuf(buffer.rdbuf());                                                               \
    action std::string capture = buffer.str();                                                     \
    std::cout.rdbuf(sbuf);                                                                         \
    EXPECT_STREQ(ExpectedOut.c_str(), buffer.str().c_str());                                       \
  }

class BtwxtContextCourierr : public Courierr::Courierr
{
  public:
  void error(const std::string_view message) override { write_message("  ERROR:", message); }
  void warning(const std::string_view message) override { write_message("  WARNING:", message); }
  void info(const std::string_view message) override { write_message("  NOTE:", message); }
  void debug(const std::string_view message) override { write_message("  DEBUG:", message); }

  private:
    void write_message(const std::string_view message_type, const std::string_view message)
    {
        std::string context_string = message_context ? *(reinterpret_cast<std::string *>(message_context)) : "";
        std::cout << fmt::format("{}{} {}", context_string, message_type, message) << std::endl;
    }
};

namespace Btwxt {

class OneDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target{2.5};

  OneDFixture() {
    std::vector<std::vector<double>> grid = {{0, 2, 5, 10}};
    std::vector<std::vector<double>> values = {{6, 5, 4, 3}};

    auto courier = std::make_shared<BtwxtContextCourierr>();
    test_gridded_data = GriddedData(grid, values, courier);
    test_rgi = RegularGridInterpolator(test_gridded_data, courier);
  }
};

class OneDL0Fixture : public testing::Test {
protected:
  std::vector<std::vector<double>> grid;
  std::vector<std::vector<double>> values;

  OneDL0Fixture() {
    grid = {{}};
    values = {{}};
  }
};

class OneDL1Fixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  std::vector<double> target{2.5};

  OneDL1Fixture() : test_rgi({{2}}, {{5}}, std::make_shared<BtwxtContextCourierr>()) { test_rgi.set_axis_extrap_method(0, Method::LINEAR); }
};

class OneDL2Fixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  std::vector<double> target;

  OneDL2Fixture() {
    std::vector<std::vector<double>> grid = {{0, 10}};
    std::vector<std::vector<double>> values = {{6, 3}};

    target = {2.5};
    test_rgi = RegularGridInterpolator(grid, values, std::make_shared<BtwxtContextCourierr>());
    test_rgi.set_axis_extrap_method(0, Method::LINEAR);
  }
};

class TwoDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target{12, 5};
  std::vector<std::vector<double>> values;
  std::shared_ptr<BtwxtContextCourierr> courier = std::make_shared<BtwxtContextCourierr>();

  TwoDFixture() {
    std::vector<std::vector<double>> grid = {{0, 10, 15}, {4, 6}};
    //         4  6
    values = {{6, 3,  // 0
               2, 8,  // 10
               4, 2}, // 15
              {12, 6, 4, 16, 8, 4}};
    test_gridded_data = GriddedData(grid, values, courier);
    test_gridded_data.set_axis_extrap_method(0, Method::LINEAR);
    test_rgi = RegularGridInterpolator(test_gridded_data, courier);
  }
};

class TwoDSimpleNormalizationFixture : public testing::Test {
  // TODO: Create a fixture which this one can inherit from
  // takes a vector of functions as a parameter (these become separate value tables)
  // takes a vector of vectors which is the data structure that stores the grid
protected:
  RegularGridInterpolator test_rgi;
  double test_function(std::vector<double> target) {
    assert(target.size() == 2);
    return target[0] * target[1];
  }

  TwoDSimpleNormalizationFixture() {
    std::vector<std::vector<double>> grid = {{2.0, 7.0}, {1.0, 2.0, 3.0}};
    std::vector<double> values;
    for (auto x : grid[0]) {
      for (auto y : grid[1]) {
        values.push_back(test_function({x, y}));
      }
    }
    test_rgi = RegularGridInterpolator(grid, {values}, std::make_shared<BtwxtContextCourierr>());
    test_rgi.set_axis_extrap_method(0, Method::LINEAR);
  }
};

class CubicFixture : public testing::Test {
protected:
  std::shared_ptr<BtwxtContextCourierr> courier = std::make_shared<BtwxtContextCourierr>();
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target;

  CubicFixture() {
    std::vector<std::vector<double>> grid = {{6, 10, 15, 20}, {2, 4, 6, 8}};
    std::vector<std::vector<double>> values =
        //  2   4   6    8
        {{4, 3, 1.5, 1, // 6
          5, 4, 2, 1,   // 10
          8, 6, 3, 2,   // 15
          10, 8, 4, 2}, // 20

         {12, 10, 4, 4,    // 6
          16, 12, 6, 4,    // 10
          20, 16, 8, 4,    // 15
          25, 20, 10, 5}}; // 20

    target = {12, 4.5};
    test_gridded_data = GriddedData(grid, values, courier);
    test_gridded_data.set_axis_interp_method(0, Method::CUBIC);
    test_rgi = RegularGridInterpolator(test_gridded_data, courier);
  }
};

class ThreeDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<std::vector<double>> values;
  std::vector<double> target{26.9, 12, 5};

  ThreeDFixture() {
    std::vector<std::vector<double>> grid = {{-15, 0.2, 105}, {0, 10, 15}, {4, 6}};
    //         4   6
    values = {{6, 3, // 0
               2, 8, // 10
               4, 2, // 15
               3, 6, 13, 2, 0, 15, 3, 6, 13, 2, 0, 15}};
    auto courier = std::make_shared<BtwxtContextCourierr>();
    test_gridded_data = GriddedData(grid, values, courier);
    test_gridded_data.set_axis_interp_method(0, Method::LINEAR);
    test_gridded_data.set_axis_interp_method(1, Method::CUBIC);
    test_gridded_data.set_axis_interp_method(2, Method::LINEAR);
    test_rgi = RegularGridInterpolator(test_gridded_data, courier);
  }
};
#endif /* TEST_FIXTURE_HPP_ */
} // namespace Btwxt