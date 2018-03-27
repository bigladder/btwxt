/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include "gtest/gtest.h"

// btwxt
#include <btwxt.h>
#include <griddeddata.h>
#include <error.h>

using namespace Btwxt;


// Unit tests on free functions, generally mathematically oriented.


TEST(GriddedData, free_check_sorted) {
  std::vector< std::pair< std::vector<double>, bool > > my_vecs =
  {
    {{1,3,5,7,9},  true},
    {{1,3,5,17,9}, false},
    {{9,7,5,3,1},  true},
    {{9,17,5,3,1}, false},
    {{1,3,3,7,9},  false},
    {{9,7,5,5,1},  false},
    {{9},          true}
  };
  bool is_sorted;
  for (auto pair : my_vecs) {
    is_sorted = Btwxt::free_check_sorted(pair.first);
    showMessage(MSG_INFO, (is_sorted? "true" : "false"));
    EXPECT_EQ(is_sorted, pair.second);
  }
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


TEST(Btwxt, index_below_in_vector) {
  std::vector<double> grid_vector = {1,3,5,7,9};
  double target = 5.3;
  std::size_t expected_floor = 2;
  std::size_t returned_floor = index_below_in_vector(target, grid_vector);
  EXPECT_EQ(returned_floor, expected_floor);
  double expected_weight = 0.15;
  double edge[2] = {grid_vector[returned_floor], grid_vector[returned_floor+1]};
  double returned_weight = compute_fraction(target, edge);
  EXPECT_DOUBLE_EQ(returned_weight, expected_weight);

  // if smaller than all numbers, return vector.size()
  target = 0.3;
  expected_floor = 5;
  returned_floor = index_below_in_vector(target, grid_vector);
  EXPECT_EQ(returned_floor, expected_floor);

  // larger than all numbers returns last index
  target = 9.3;
  expected_floor = 4;
  returned_floor = index_below_in_vector(target, grid_vector);
  EXPECT_EQ(returned_floor, expected_floor);
}

TEST(Btwxt, compute_fraction) {
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
