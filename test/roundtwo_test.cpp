/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include "gtest/gtest.h"

// btwxt
#include <roundtwo.h>

using namespace Btwxt;


class OneDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  std::vector<double> target;

  OneDFixture(){
    std::vector<std::vector<double> > grid = { {0, 10} };
    std::vector<std::vector<double> > values = { {6, 3} };

    target = {4};
    test_rgi = RegularGridInterpolator(grid, values);
  }
};

class TwoDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  std::vector<double> target;

  TwoDFixture(){
    std::vector<std::vector<double> > grid = { {0, 10, 15}, {4, 6} };
    std::vector<std::vector<double> > values =
      { {6, 3, 2, 8, 4, 2},
        {1, 1, 1, 1, 1, 1} };

    target = {4, 5};
    test_rgi = RegularGridInterpolator(grid, values);
  }
};


TEST_F(OneDFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 1);

  test_rgi.set_new_grid_point(target);
};

TEST_F(TwoDFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 2);

  test_rgi.set_new_grid_point(target);
};


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  std::cout << "Hello World" << std::endl;

  return RUN_ALL_TESTS();
};
