/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#include "longtable.h"

namespace Interp {


LongTable::LongTable()
{

}

LongTable::LongTable(
  std::vector< std::vector<double> > grid,
  const double* values) :
grid(grid),
values(values)  // this is an initializer list, apparently
{
    ndims = grid.size();
    nvalues = 1;
    for(auto dim : grid){
      nvalues *= dim.size();
    }
}

}
