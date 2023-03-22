/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

#include "grid-axis.h"
#include "regular-grid-interpolator.h"

class BtwxtException : public Courierr::CourierrException {
public:
  explicit BtwxtException(const char* message, Courierr::Courierr& logger)
      : CourierrException(message, logger)
  {
  }
  explicit BtwxtException(const std::string& message, Courierr::Courierr& logger)
      : CourierrException(message, logger)
  {
  }
  explicit BtwxtException(const std::string_view message, Courierr::Courierr& logger)
      : CourierrException(message, logger)
  {
  }
};
