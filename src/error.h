/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef BTWXT_ERROR_H_
#define BTWXT_ERROR_H_

// BTWXT
#include "roundtwo.h"

namespace Btwxt {


const int MSG_INFO = 0;
const int MSG_WARN = 1;
const int MSG_ERR = 2;

typedef void(*BtwxtCallbackFunction)(
  const int messageType,
  const std::string message,
  void* contextPtr
  );


extern BtwxtCallbackFunction btwxtCallbackFunction;
extern void* messageCallbackContextPtr;

void showMessage(
  const int messageType,
  const std::string message
);

void setMessageCallback(
  BtwxtCallbackFunction callbackFunction,
  void* contextPtr
);

}
#endif // BTWXT_ERROR_H_
