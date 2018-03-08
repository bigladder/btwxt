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
      dim_lengths.push_back(dim.size());
      nvalues *= dim.size();
    }
}

std::size_t LongTable::get_ndims()
{
  return ndims;
}

std::size_t LongTable::get_nvalues()
{
  return nvalues;
}

std::vector<size_t> LongTable::get_dim_lengths()
{
  return dim_lengths;
}

// TODO: delete. n-dim upgrade now exists below
double LongTable::get_value(std::size_t c1, std::size_t c2)
{
  return values[c2*grid[0].size() + c1];
}

double LongTable::get_value_gen(std::vector<size_t> x)
{
  std::size_t index = x[0];
  if (x.size() < 2) {
    return values[index];
  }
  else {
    for (std::size_t j=1; j<x.size(); j++) {
      std::size_t p = x[j];
      for (std::size_t k=0; k<j; k++) {
        p *= dim_lengths[k];
      }
      index += p;
    }
    return values[index];
  }
}

// TODO: do something smarter if on the grid point
std::size_t LongTable::grid_floor(double x, std::size_t dim)
{
  std::vector<double>::iterator upper;
  upper = std::upper_bound(grid[dim].begin(), grid[dim].end(), x);
  std::size_t floor = upper - grid[dim].begin() - 1;
  return floor;
}

double LongTable::get_fraction(double x, std::size_t dim)
{
  std::size_t floor = LongTable::grid_floor(x, dim);
  double frac = (x - grid[dim][floor]) / (grid[dim][floor+1] -grid[dim][floor]);
  return frac;
}

}
