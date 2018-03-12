// /* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
// * See the LICENSE file for additional terms and conditions. */
//
// // btwxt
// #include "interp.h"
// #include "longtable.h"
//
//
// namespace Interp{
//
//
// // some free functions
// double interpolate(double t, double a0, double a1) {
//   // general linear interpolation in one dimension
//   return t*a1 + (1-t)*a0;
// }
//
// std::size_t pow(std::size_t base, std::size_t power) {
//   // raise base to a power (both must be size_t)
//   if (power == 0) {return 1;}
//   else {
//     std::size_t result = base;
//     for (std::size_t i=1; i<power; i++)
//     {
//         result = result*base;
//     }
//     return result;
//   }
// }
//
//
// Interp::Interp()
// {
//
// };
//
// Interp::Interp(
//   std::vector< std::vector<double> > grid,
//   std::vector<double> values,
//   std::vector<double> target) :
// target(target)
// {
//     longtable = LongTable(grid, values);
//     ndims = longtable.get_ndims();
//     nvalues = longtable.get_nvalues();
//     set_floors();
// };
//
// std::size_t Interp::get_ndims()
// {
//   return ndims;
// }
//
// std::size_t Interp::get_nvalues()
// {
//   return nvalues;
// }
//
// std::vector<std::size_t> Interp::get_floors()
// {
//   return floors;
// }
//
// void Interp::set_floors()
// {
//   // std::vector<std::size_t> floors;
//   // floors = {3, 2};
//   // floors.push_back(3);
//   // floors.push_back(longtable.grid_floor(20, 0));
//   for (std::size_t d=0; d<ndims; d+=1) {
//     floors.push_back(longtable.grid_floor(target[d], d));
//   }
// }
//
// }
