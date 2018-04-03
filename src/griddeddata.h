/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDDEDDATA_H_
#define GRIDDEDDATA_H_

#include<vector>
#include "Eigen/Dense"



namespace Btwxt{

const int CON_EXTR = 0;  // constant extrapolation
const int LIN_EXTR = 1;  // linear extrapolation

class GridAxis{
  // A single input dimension of the performance space
public:
  GridAxis();
  GridAxis(double *grid, std::size_t size, int extrapolation_method = CON_EXTR);
  GridAxis(std::vector<double> grid_vector,
    int extrapolation_method = CON_EXTR);

  const std::vector<double> grid;
  int extrapolation_method;
  // std::pair<double> extrapolation_bounds;  <-- to add later
  // bool is_regular;  <-- to add later

  std::size_t get_length();
};


class GridSpace{
public:
  // all n input dimensions to specify the performance space
  GridSpace();
  GridSpace(std::vector<GridAxis> grid_axes);

  std::vector<GridAxis> axes;

  std::size_t get_ndims();
  std::size_t get_dim_length(const std::size_t&);
  std::vector<std::size_t> get_dim_lengths();
};




class GriddedData{
public:
  GriddedData();
  GriddedData(
    std::vector< std::vector<double> > grid,
    std::vector< std::vector<double> > values
  );

  std::size_t get_ndims();
  std::size_t get_num_tables();
  std::vector<double> get_grid_vector(const std::size_t& grid_index);
  std::vector<double> get_values(const std::vector<std::size_t>& coords);
  Eigen::ArrayXd get_column(const std::vector<std::size_t>& coords);
  int get_axis_extrap_method(const std::size_t& grid_index);
  // double get_value(std::size_t table_index, std::vector<std::size_t> coords);

  void set_axis_extrap_method(const std::size_t& grid_index, const int);
  Eigen::ArrayXXd value_tables;
  std::size_t num_values;
  std::vector<std::size_t> dimension_lengths;

private:
  void construct_axes(const std::vector< std::vector<double> >& grid);
  Eigen::ArrayXXd construct_values(
    const std::vector< std::vector<double> >& values
  );

  GridSpace grid_axes;
  std::size_t ndims;
  std::size_t num_tables;
};


// free functions
bool free_check_sorted(std::vector<double>);
std::size_t locate_coords(
  const std::vector<std::size_t>& coords,
  const std::vector<std::size_t>& dimension_lengths
);
std::vector<double> eigen_to_vector(Eigen::ArrayXd&);


}
#endif // GRIDDEDDATA_H_
