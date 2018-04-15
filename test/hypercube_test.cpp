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
