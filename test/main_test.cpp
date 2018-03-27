/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


#include "gtest/gtest.h"

// btwxt
#include <btwxt.h>
#include <griddeddata.h>
#include <error.h>


using namespace Btwxt;


// use custom callback function b/c we don't want to exit tests on error.
void my_callback(const int messageType,
  const std::string message, void* contextPtr);



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


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  int* my_context_ptr;
  setMessageCallback(my_callback, my_context_ptr);

  showMessage(MSG_INFO, "Hello World");

  return RUN_ALL_TESTS();
};
