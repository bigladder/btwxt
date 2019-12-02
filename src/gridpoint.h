/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef GRIDPOINT_H_
#define GRIDPOINT_H_

// Standard
#include <map>
#include <memory>
#include <vector>

// btwxt
#include "griddeddata.h"

namespace Btwxt
{

enum class Bounds
{
  OUTLAW,
  OUTBOUNDS,
  INBOUNDS
};

/// @class GridPoint gridpoint.h
/// @brief This class performs the N-dimensional interpolation for the desired N-dimensional target
///        point.

class GridPoint
{
public:
  GridPoint();

  GridPoint(GriddedData &grid_data);

  GridPoint(GriddedData &grid_data, std::vector<double> v);

  void set_target(const std::vector<double> &v);

  std::vector<double> get_current_target();

  std::vector<std::size_t> get_floor();

  std::vector<double> get_weights();

  std::vector<Bounds> get_is_inbounds();

  std::vector<Method> get_methods();

  std::vector<std::vector<double>> get_interp_coeffs();

  std::vector<std::vector<double>> get_cubic_slope_coeffs();

  std::vector<double> get_results();

  double get_vertex_weight(const std::vector<short> &v);

  void normalize_grid_values_at_target(std::size_t table_num, const double scalar = 1.0);

  void normalize_grid_values_at_target(const double scalar = 1.0);

  void set_floor();

private:
  friend class RegularGridInterpolator;
  GriddedData *grid_data;     ///< Local pointer to externally-defined GriddedData object
  std::size_t ndims;          ///< Local copy of the number of GriddedData dimensions
  std::vector<double> target; ///< One number for each of the GriddedData dimensions
  bool target_is_set;
  std::vector<std::size_t> point_floor; ///< N-dimensional coordinate of the nearest
                                        ///< grid point <= target
  std::size_t floor_index;              ///< Index of the point_floor coordinate in value-space
  std::vector<double> weights;     ///< N-dimensional weight calculated as the ratio between the
                                   ///< point_floor-to-target distance, and point_floor-to-adjacent
                                   ///< point distance
  std::vector<Bounds> is_inbounds; ///< for deciding interpolation vs. extrapolation when
                                   ///< there is a change in the target
  std::vector<Method> methods;     ///< Extrapolation or interpolation method
  std::vector<Method> previous_methods;      ///< Temporary; does not need to be a member
  std::vector<std::vector<short>> hypercube; ///< An ordered list of relative indices
                                             ///< necessary for the interpolation of target.
                                             ///< Size = Product(N_nodes for each axis'
                                             ///< interpolation)
  bool reset_hypercube;                      ///< If the target changes? Or interp method changes?
  std::vector<std::vector<double>> weighting_factors; ///< A set of 4 weighting factors for each
                                                      ///< dimension

  std::vector<std::vector<double>> interp_coeffs;      ///< Coefficients for calculating vertex
                                                       ///< weights
  std::vector<std::vector<double>> cubic_slope_coeffs; ///< Coefficients for calculating vertex
                                                       ///< weights

  std::vector<std::vector<double>> hypercube_values; ///< Performance map values corresponding to
                                                     ///< the placement of the hypercube origin
                                                     ///< at the point floor
  std::vector<double> hypercube_weights;             ///< No idea. Same size as hypercube
  std::vector<double> results; ///< Results of the interpolation for each target

  void calculate_weights();

  void consolidate_methods();

  void calculate_interp_coeffs();

  void set_dim_floor(std::size_t dim);

  void set_hypercube();

  void set_hypercube(std::vector<Method> methods);

  std::vector<std::vector<short>> &get_hypercube();

  void set_hypercube_values();

  void set_results();

  std::map<std::pair<std::size_t, std::size_t>, std::vector<std::vector<double>>>
      hypercube_cache; ///< Once a set of @c hypercube_values is calculated for any given point
                       ///< floor, it's cached here using a key consisting of two unique numbers:
                       ///< {floor_index, hash comprised of each dimension's interpolation method}

  std::size_t hypercube_size_hash; ///< integer hash comprised of digits representing each
                                   ///< dimension's interpolation method
};

// free functions
double compute_fraction(double x, double edge[2]);
} // namespace Btwxt

#endif // GRIDPOINT_H_
