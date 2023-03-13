/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef BTWXT_ERROR_H_
#define BTWXT_ERROR_H_

#include <functional>
#include <sstream>
#include <memory>

#include <courierr/courierr.h>

namespace Btwxt {

struct expand_type {
  template <typename... T> expand_type(T &&...) {}
};

template <typename... ArgTypes> std::string stringify(ArgTypes... args) {
  std::ostringstream oss;
  expand_type{0, (oss << args, 0)...};
  return oss.str();
}

class BtwxtErr : public Courierr::CourierrException {
  public:
    explicit BtwxtErr(const char* message, Courierr::Courierr& logger)
     : CourierrException(message, logger)
    {
    }
    explicit BtwxtErr(const std::string& message, Courierr::Courierr& logger)
     : CourierrException(message, logger)
    {
    }
    explicit BtwxtErr(const std::string_view message, Courierr::Courierr& logger)
     : CourierrException(message, logger)
    {
    }
};

} // namespace Btwxt
#endif // BTWXT_ERROR_H_
