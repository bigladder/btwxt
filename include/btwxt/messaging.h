/* Copyright (c) 2023 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef BTWXT_MESSAGING_H_
#define BTWXT_MESSAGING_H_

#include <fmt/format.h>

#include <courier/courier.h>
#include <courier/helpers.h>

namespace Btwxt {

class BtwxtDefaultCourier : public Courier::DefaultCourier {
  protected:
    void write_message(const std::string& message_type, const std::string& message) override
    {
        std::cout << fmt::format("  [{}] {}", message_type, message) << std::endl;
        std::cout
            << "  Generated using BtwxtDefaultCourier. Consider deriving your own Courier class!"
            << std::endl;
    }
};

} // namespace Btwxt

#endif // define BTWXT_MESSAGING_H_