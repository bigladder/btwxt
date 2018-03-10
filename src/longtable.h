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
  std::size_t get_ndims();
  std::size_t get_nvalues();
  std::vector<size_t> get_dim_lengths();
  double get_value_2d(std::size_t, std::size_t);
  double get_value(std::vector<size_t> x);
  size_t grid_floor(double, std::size_t);
  double get_fraction(double, std::size_t);
private:
  std::size_t ndims;
  std::size_t nvalues;
  std::vector<size_t> dim_lengths;
};

}
#endif // LONGTABLE_H_
