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
  } else {
    std::string full_message (message);
    if (messageType == Btwxt::MSG_ERR) {
      full_message.insert(0, "  ERROR: ");
      std::cout << full_message << std::endl;
      exit(EXIT_FAILURE);
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
