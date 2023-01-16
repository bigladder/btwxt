/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef BTWXT_ERROR_H_
#define BTWXT_ERROR_H_

#include <sstream>

// BTWXT
#include "btwxt.h"

namespace Btwxt {

void showMessage(MsgLevel messageType, std::string message);

struct expand_type {
  template <typename... T> expand_type(T &&...) {}
};

template <typename... ArgTypes> std::string stringify(ArgTypes... args) {
  std::ostringstream oss;
  expand_type{0, (oss << args, 0)...};
  return oss.str();
}

class BtwxtException : public std::exception {
public:

  explicit BtwxtException(const char *message) : msg_(message) {}

  explicit BtwxtException(const std::string &message) : msg_(message) {}

  virtual ~BtwxtException() noexcept = default;

  virtual const char* what() const noexcept { return msg_.c_str(); }

protected:
  std::string msg_;
};

class BtwxtInfo : public BtwxtException {
public:
  explicit BtwxtInfo(const char *message) : BtwxtException(message) {}

  explicit BtwxtInfo(const std::string &message) : BtwxtException(message) {}

  ~BtwxtInfo() noexcept = default;
};

class BtwxtWarn : public BtwxtException {
public:
  explicit BtwxtWarn(const char *message) : BtwxtException(message) {}

  explicit BtwxtWarn(const std::string &message) : BtwxtException(message) {}

  ~BtwxtWarn() noexcept = default;
};

class BtwxtErr : public BtwxtException {
public:
  explicit BtwxtErr(const char *message) : BtwxtException(message) {}

  explicit BtwxtErr(const std::string &message) : BtwxtException(message) {}

  ~BtwxtErr() noexcept = default;
};

} // namespace Btwxt
#endif // BTWXT_ERROR_H_
