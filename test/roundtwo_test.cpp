/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include "gtest/gtest.h"

// btwxt
#include <roundtwo.h>
#include <error.h>


using namespace Btwxt;


// use custom callback function b/c we don't want to exit tests on error.
void my_callback(const int messageType,
  const std::string message, void* contextPtr);
template<typename T>
void make_array_message_str(std::string&, T[], std::size_t);


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
  GriddedData test_gridded_data;
  std::vector<double> target;

  TwoDFixture(){
    std::vector<std::vector<double> > grid = { {0, 10, 15}, {4, 6} };
    std::vector<std::vector<double> > values =
      { {6, 3, 2, 8, 4, 2},
        {12, 6, 4, 16, 8, 4} };

    target = {4, 5};
    test_gridded_data = GriddedData(grid, values);
    test_rgi = RegularGridInterpolator(test_gridded_data);
  }
};

class MismatchedFixture : public testing::Test {
protected:
  RegularGridInterpolator test_rgi;
  std::vector<double> target;

  MismatchedFixture(){
    std::vector<std::vector<double> > grid = { {0, 17, 15}, {4, 6} };   // not sorted
    std::vector<std::vector<double> > values =
      { {6, 3, 2, 8, 4},          // five items
        {1, 1, 1, 1, 1, 1, 1} };  // seven items

    target = {4, 5, 6};           // three dimensions
    test_rgi = RegularGridInterpolator(grid, values);
  }
};



TEST_F(OneDFixture, construct_from_vectors) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 1);
};

TEST_F(TwoDFixture, construct_from_gridded_data) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 2);
};

TEST_F(TwoDFixture, get_value) {
  std::vector<std::size_t> coords = {0, 1};
  double returned_value = test_gridded_data.get_value(0, coords);
  EXPECT_EQ(returned_value, 8);

  coords = {1, 0};
  returned_value = test_gridded_data.get_value(1, coords);
  EXPECT_EQ(returned_value, 6);

  // should return 0 and a warning that we don't have that many tables
  returned_value = test_gridded_data.get_value(2, coords);
  EXPECT_EQ(returned_value, 0);

  // should return 0 and a warning that we overran dimension 0
  coords = {7, 0};
  returned_value = test_gridded_data.get_value(1, coords);
  EXPECT_EQ(returned_value, 0);
};

TEST_F(TwoDFixture, return_target) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  std::vector<double> returned_target = test_rgi.get_current_grid_point();
  for (std::size_t i=0; i<ndims; i++) {
    EXPECT_EQ(target[i], returned_target[i]);
  }
};

TEST_F(MismatchedFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();

  // TODO: capture error messages and test that they match expectations
  // we expect two errors that the value table inputs do not match the grid
  // we expect an error that the target dimensions do not match the grid
  test_rgi.set_new_grid_point(target);
};

TEST(CheckSorted, free_check_sorted) {
  std::vector< std::pair< std::vector<double>, bool > > my_vecs =
  {
    {{1,3,5,7,9}, true},
    {{1,3,5,17,9}, false},
    {{9,7,5,3,1}, true},
    {{9,17,5,3,1}, false},
    {{1,3,3,7,9}, false},
    {{9,7,5,5,1}, false},
    {{9}, true}
  };
  bool is_sorted;
  for (auto pair : my_vecs) {
    is_sorted = Btwxt::free_check_sorted(pair.first);
    showMessage(MSG_INFO, (is_sorted? "true" : "false"));
    EXPECT_EQ(is_sorted, pair.second);
  }
};

void my_callback(
  const int messageType,
  const std::string message,
  void* contextPtr
)
{
  if (messageType == Btwxt::MSG_INFO) {
    std::cout << "  NOTE: " << message << std::endl;
  } else if (messageType == Btwxt::MSG_WARN) {
    std::cout << "  WARNING: " << message << std::endl;
  } else if (messageType == Btwxt::MSG_ERR) {
    std::cout << "  ERROR: " << message << std::endl;
  }
}

template<typename T>
void make_array_message_str(std::string &message_str,
                            T items[], std::size_t num_items)
{
  bool first_item = true;
  for (std::size_t i=0; i<num_items; i++) {
    message_str += (first_item ? "{" : ", ");
    message_str += std::to_string(items[i]);
    first_item = false;
  }
  message_str += "}";
};


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  int* my_context_ptr;
  setMessageCallback(my_callback, my_context_ptr);

  showMessage(MSG_INFO, "Hello World");

  return RUN_ALL_TESTS();
};
