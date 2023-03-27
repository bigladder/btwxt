/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#pragma once

// Standard
#include <algorithm>
#include <cfloat>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

// btwxt
#include "logging.h"

namespace Btwxt {

class GridPointData {
    // Data corresponding to all points within a collection of grid axes. Length of data should
    // equal the total number of permutations of grid axes points.
  public:
    // Constructors
    GridPointData() = default;

    explicit GridPointData(std::vector<double> data, const std::string& name = "")
        : data(std::move(data)), name(name)
    {
    }
    std::vector<double> data;

    // Setters
    void set_name(const std::string& name_in) { name = name_in; }

    // Getters
    const std::string& set_name() { return name; }

  private:
    std::string name;
};

} // namespace Btwxt