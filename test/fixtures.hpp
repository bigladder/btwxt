/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


#ifndef TEST_FIXTURE_HPP_
#define TEST_FIXTURE_HPP_


#include "gtest/gtest.h"


// btwxt
#include <btwxt.h>
#include <griddeddata.h>
#include <error.h>


using namespace Btwxt;


class OneDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target;

  OneDFixture(){
    std::vector<std::vector<double> > grid = { {0, 10} };
    std::vector<std::vector<double> > values = { {6, 3} };

    target = {4};
    test_gridded_data = GriddedData(grid, values);
    test_rgi = RegularGridInterpolator(test_gridded_data);
  }
};

class TwoDFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target;

  TwoDFixture(){
    std::vector<std::vector<double> > grid = { {0, 10, 15}, {4, 6} };
    std::vector<std::vector<double> > values =
      { {6, 3, 2, 8, 4, 2},
        {12, 6, 4, 16, 8, 4} };

    target = {12, 5};
    test_gridded_data = GriddedData(grid, values);
    test_gridded_data.set_axis_extrap_method(0, LIN_EXTR);
    test_rgi = RegularGridInterpolator(test_gridded_data);
  }
};

class MismatchedFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target;

  MismatchedFixture(){
    std::vector<std::vector<double> > grid = { {0, 17, 15}, {4, 6} };   // not sorted
    std::vector<std::vector<double> > values =
      { {6, 3, 2, 8, 4},          // five items
        {1, 1, 1, 1, 1, 1, 1} };  // seven items

    target = {4, 5, 6};           // three dimensions
    test_gridded_data = GriddedData(grid, values);
    test_rgi = RegularGridInterpolator(test_gridded_data);
  }
};


class CubicFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  GriddedData test_gridded_data;
  std::vector<double> target;

  CubicFixture(){
    std::vector<std::vector<double> > grid = { {6, 10, 15, 20}, {4, 6} };
    std::vector<std::vector<double> > values =
      { {8, 6, 3, 2, 10, 8, 4, 2},
        {16, 12, 6, 4, 20, 16, 8, 4} };

    target = {12, 4};
    test_gridded_data = GriddedData(grid, values);
    test_gridded_data.set_axis_interp_method(0, CUB_INTR);
    test_rgi = RegularGridInterpolator(test_gridded_data);
  }
};

#endif /* TEST_FIXTURE_HPP_ */
