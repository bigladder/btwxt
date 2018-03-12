// /* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
// * See the LICENSE file for additional terms and conditions. */
//
// #ifndef INTERP_H_
// #define INTERP_H_
//
// // Standard
// #include <vector>
// #include "longtable.h"
//
//
// namespace Interp {
//
// class Interp{
// public:
//   Interp();
//   // Interp(
//   //   std::vector< std::vector<double> > grid,
//   //   const double* values,
//   //   std::vector<double> target);
//   Interp(
//     std::vector< std::vector<double> > grid,
//     std::vector<double> values,
//     std::vector<double> target);
//
//   std::vector<double> target;
//   LongTable longtable;
//
//   void set_floors();
//   std::size_t get_ndims();
//   std::size_t get_nvalues();
//   std::vector<std::size_t> get_floors();
//
// private:
//   std::vector<std::size_t> floors;
//   std::size_t ndims;
//   std::size_t nvalues;
//
// };
//
// double interpolate(double, double, double);
// std::size_t pow(std::size_t, std::size_t);
//
// }
// #endif // INTERP_H_
