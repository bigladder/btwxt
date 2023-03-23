/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>

// btwxt
#include <btwxt/btwxt.h>

// testing
#include "public-fixtures.hpp"

namespace Btwxt {

TEST(GridAxis, free_check_sorted) {
  std::vector<std::pair<std::vector<double>, bool>> my_vecs = {{{1, 3, 5, 7, 9}, true},
                                                               {{1, 3, 5, 17, 9}, false},
                                                               {{9, 7, 5, 3, 1}, false},
                                                               {{1, 3, 3, 7, 9}, false},
                                                               {{9}, true}};
  bool is_sorted;
  for (auto pair : my_vecs) {
    is_sorted = Btwxt::free_check_sorted(pair.first);
    EXPECT_EQ(is_sorted, pair.second);
  }
}

TEST(GridAxis, sorting) {
  std::vector<double> grid_vector = {0, 5, 7, 17, 15};
  auto courier = std::make_shared<BtwxtContextCourierr>();
  EXPECT_THROW(GridAxis my_grid_axis = GridAxis(grid_vector, courier), BtwxtException);
  grid_vector = {0, 5, 7, 10, 15};
  EXPECT_NO_THROW(GridAxis my_grid_axis = GridAxis(grid_vector, courier););
}

TEST(GridAxis, calc_spacing_multipliers) {
  std::vector<double> grid_vector{6, 10, 15, 20, 22};

  GridAxis test_gridaxis(grid_vector, std::make_shared<BtwxtContextCourierr>(), Method::CONSTANT,
                         Method::CUBIC, {-DBL_MAX, DBL_MAX});
  std::vector<std::vector<double>> values = test_gridaxis.spacing_multipliers;
  EXPECT_THAT(values[0], testing::ElementsAre(1, 5.0 / 9, 0.5, 2.0 / 7));
  EXPECT_THAT(values[1], testing::ElementsAre(4.0 / 9, 0.5, 5.0 / 7, 1));
}

TEST(GridAxis, bad_limits) {
  auto courier = std::make_shared<BtwxtContextCourierr>();
  GridAxis my_grid_axis({0, 5, 7, 11, 12, 15}, courier);
  std::pair<double, double> extrap_limits{4, 17};
  std::string ExpectedOut = "  NOTE: The lower extrapolation limit (4) is within the set of "
                            "grid values. Setting to smallest grid value (0).\n";
  EXPECT_STDOUT(my_grid_axis.set_extrap_limits(extrap_limits);, ExpectedOut);
  EXPECT_EQ(my_grid_axis.extrapolation_limits.first, 0);

  extrap_limits = {-2, 12};
  ExpectedOut = "  NOTE: The upper extrapolation limit (12) is within the set of grid values. "
                "Setting to largest grid value (15).\n";
  EXPECT_STDOUT(my_grid_axis.set_extrap_limits(extrap_limits);, ExpectedOut);
  EXPECT_EQ(my_grid_axis.extrapolation_limits.second, 15);
}
} // namespace Btwxt