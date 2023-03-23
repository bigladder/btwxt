/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

#include <memory>

#include "gtest/gtest.h"

// btwxt
#include "../src/regular-grid-interpolator-private.h"
#include <btwxt/btwxt.h>

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

class BtwxtContextCourierr : public Courierr::Courierr {
public:
  void error(const std::string_view message) override { write_message("  ERROR:", message); }
  void warning(const std::string_view message) override { write_message("  WARNING:", message); }
  void info(const std::string_view message) override { write_message("  NOTE:", message); }
  void debug(const std::string_view message) override { write_message("  DEBUG:", message); }

private:
  void write_message(const std::string_view message_type, const std::string_view message) {
    std::string context_string =
        message_context ? *(reinterpret_cast<std::string *>(message_context)) : "";
    std::cout << fmt::format("{}{} {}", context_string, message_type, message) << std::endl;
  }
};

namespace Btwxt {

class GridFixture : public testing::Test {
public:
  std::vector<std::vector<double>> grid;
  std::vector<std::vector<double>> values;
  std::vector<double> target;
  RegularGridInterpolator interpolator;

  GridFixture() {}

  void setup() {
    interpolator = RegularGridInterpolator(grid, values, std::make_shared<BtwxtContextCourierr>());
  }
};

class GridFixture2D : public GridFixture {
protected:
  GridFixture2D() {
    grid = grid = {{0, 10, 15}, {4, 6}};
    //         4  6
    values = {{6, 3,   // 0
               2, 8,   // 10
               4, 2},  // 15
              {12, 6,  // 0
               4, 16,  // 10
               8, 4}}; // 15
    target = {12, 5};
    setup();
    interpolator.set_axis_extrap_method(0, Method::LINEAR);
  }
};

class FunctionFixture : public GridFixture {
public:
  std::vector<std::function<double(std::vector<double>)>> functions;

protected:
  FunctionFixture() {}

  void setup() {
    values.resize(functions.size());
    for (std::size_t i = 0u; i < functions.size(); i++) {
      for (auto &grid_point : cart_product(grid)) {
        values[i].push_back(functions[i](grid_point));
      }
    }
    interpolator = RegularGridInterpolator(grid, values, std::make_shared<BtwxtContextCourierr>());
  }
};

class FunctionFixture2D : public FunctionFixture {
protected:
  FunctionFixture2D() {
    grid = {{2.0, 7.0}, {1.0, 2.0, 3.0}};
    functions = {[](std::vector<double> x) -> double { return x[0] * x[1]; }};
    setup();
  }
};

} // namespace Btwxt