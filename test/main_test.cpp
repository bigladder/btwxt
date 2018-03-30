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
  std::string full_message (message);
  if (messageType == Btwxt::MSG_ERR) {
    full_message.insert(0, "  ERROR: ");
  } else if (messageType == Btwxt::MSG_WARN) {
    full_message.insert(0, "  WARNING: ");
  } else if (messageType == Btwxt::MSG_INFO) {
    full_message.insert(0, "  NOTE: ");
  } else  /*if (messageType == Btwxt::MSG_DEBUG)*/ {
    full_message.insert(0, "  DEBUG: ");
  }

  if (messageType >= Btwxt::LOG_LEVEL) {
    std::cout << full_message << std::endl;
  }
}


int main(int argc, char **argv)
{
  Btwxt::LOG_LEVEL = 1;
  ::testing::InitGoogleTest(&argc, argv);

  int* my_context_ptr;
  setMessageCallback(my_callback, my_context_ptr);

  showMessage(MSG_INFO, "Hello World");

  return RUN_ALL_TESTS();
};
