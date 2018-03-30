/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>

// Btwxt
#include "error.h"

namespace Btwxt {

BtwxtCallbackFunction btwxtCallbackFunction;
void* messageCallbackContextPtr;
int LOG_LEVEL = 2;

void showMessage(
  const int messageType,
  const std::string message
)
{
  if (btwxtCallbackFunction != NULL) {
    (*btwxtCallbackFunction)(messageType, message, messageCallbackContextPtr);
  }
  else if (messageType == Btwxt::MSG_ERR) {
    std::cout << "  ERROR: " << message << std::endl;
    exit(EXIT_FAILURE);
  } else {
    if (messageType >= Btwxt::LOG_LEVEL) {
      std::string prefix ("  DEBUG: ");
      if (messageType == Btwxt::MSG_WARN) {
        prefix = "  WARNING: ";
      } else if (messageType == Btwxt::MSG_INFO) {
        prefix = "  NOTE: ";
      }
      std::cout << prefix << message << std::endl;
    }
  }
}

void setMessageCallback(
  BtwxtCallbackFunction callBackFunction,
  void* contextPtr
)
{
  btwxtCallbackFunction = callBackFunction;
  messageCallbackContextPtr = contextPtr;
};


}
