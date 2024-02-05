/* Copyright (c) 2023 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef BTWXT_LOGGING_H_
#define BTWXT_LOGGING_H_

#include <fmt/format.h>

#include <courier/courier.h>

namespace Btwxt {

class BtwxtDefaultCourier : public Courier::Courier {
  protected:
    void receive_error(const std::string& message) override { write_message("ERROR", message); }

    void receive_warning(const std::string& message) override { write_message("WARNING", message); }

    void receive_info(const std::string& message) override { write_message("NOTE", message); }

    void receive_debug(const std::string& message) override { write_message("DEBUG", message); }

    virtual void write_message(const std::string& message_type, const std::string& message)
    {
        std::cout << fmt::format("  [{}] {}", message_type, message) << std::endl;
    }
};

} // namespace Btwxt

#endif // define BTWXT_LOGGING_H_