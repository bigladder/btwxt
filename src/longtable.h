/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef LONGTABLE_H_
#define LONGTABLE_H_

// Standard
#include <vector>

namespace Interp {

class LongTable{
public:
  LongTable();
  LongTable(
    std::vector< std::vector<double> > grid,
    const double* values);
  std::vector< std::vector<double> > grid;
  const double* values;
private:
  std::size_t ndims;
  std::size_t nvalues;
};

}
#endif // LONGTABLE_H_
