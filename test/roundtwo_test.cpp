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



TEST_F(OneDFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 1);
};

TEST_F(TwoDFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 2);
};

TEST_F(TwoDFixture, return_target) {
  std::size_t ndims = test_rgi.get_ndims();
  test_rgi.set_new_grid_point(target);

  std::vector<double> returned_target = test_rgi.get_coordinates_of_current_grid_point();
  for (std::size_t i=0; i<ndims; i++) {
    EXPECT_EQ(target[i], returned_target[i]);
  }
};

TEST_F(MismatchedFixture, ndims) {
  std::size_t ndims = test_rgi.get_ndims();
  // TODO: capture error messages and test that they match expectations

  test_rgi.set_new_grid_point(target);
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
