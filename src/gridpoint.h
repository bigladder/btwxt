/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

#ifndef GRIDPOINT_H_
#define GRIDPOINT_H_

// Standard
#include <map>
#include <memory>
#include <optional>
#include <vector>

// btwxt
#include "griddeddata.h"

namespace Courierr { class CourierrBase; }

namespace Btwxt {

enum class Bounds { OUTLAW, OUTBOUNDS, INBOUNDS };


class GridPoint {
public:

  GridPoint() = default;

  GridPoint(GriddedData &grid_data, std::shared_ptr<Courierr::CourierrBase> logger = nullptr);

  GridPoint(GriddedData &grid_data, std::vector<double> v, std::shared_ptr<Courierr::CourierrBase> logger = nullptr);

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

  void normalize_grid_values_at_target(const double scalar = 1.0);

  double normalize_grid_values_at_target(std::size_t table_num, const double scalar = 1.0);

  void set_floor();

  void set_logger(std::shared_ptr<Courierr::CourierrBase> logger) { gridpoint_logger = logger; }

private:
  friend class RegularGridInterpolator;
  friend class ThreeDGriddedDataFixture;
  friend class ThreeDGriddedDataFixture_hypercube_Test;
  friend class ThreeDGriddedDataFixture_test_hypercube_Test;
  friend class ThreeDGriddedDataFixture_make_linear_hypercube_Test;

  GriddedData *grid_data;
  std::size_t ndims;
  std::vector<double> target;
  bool target_is_set;
  std::vector<std::size_t> point_floor; // index of grid point <= target
  std::size_t floor_index;
  std::vector<double> weights;
  std::vector<Bounds> is_inbounds; // for deciding interpolation vs. extrapolation;
  std::vector<Method> methods;
  std::vector<Method> previous_methods;
  std::vector<std::vector<short>> hypercube;
  bool reset_hypercube;
  std::vector<std::vector<double>>
      weighting_factors; // A set of weighting factors for each dimension

  std::vector<std::vector<double>> interp_coeffs;
  std::vector<std::vector<double>> cubic_slope_coeffs;

  std::vector<std::vector<double>> hypercube_values;
  std::vector<double> hypercube_weights;
  std::vector<double> results;

  std::map<std::pair<std::size_t, std::size_t>, std::vector<std::vector<double>>> hypercube_cache;

  std::size_t hypercube_size_hash;

  std::shared_ptr<Courierr::CourierrBase> gridpoint_logger;

  void log_err(const std::string_view msg);

  void log_warn(const std::string_view msg);

  void log_info(const std::string_view msg);

  void calculate_weights();

  void consolidate_methods();

  void calculate_interp_coeffs();

  void set_dim_floor(std::size_t dim);

  void set_hypercube();

  void set_hypercube(std::vector<Method> methods);

  std::vector<std::vector<short>> &get_hypercube();

  void set_hypercube_values();

  void set_results();
 
};

// free functions
double compute_fraction(double x, double edge[2]);
} // namespace Btwxt

#endif // GRIDPOINT_H_
