/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDDEDDATA_H_
#define GRIDDEDDATA_H_

#include<vector>
#include "Eigen/Dense"

// We will eventually split this out into two header files,
//     the public API and the private members.


namespace Btwxt{

class GridAxis{
  // A single input dimension of the performance space
public:
  GridAxis();
  GridAxis(double *grid, std::size_t size);
  GridAxis(std::vector<double> grid_vector);
  // ~GridAxis();

  std::vector<double> grid;
  // bool is_regular;  <-- to add later

  std::size_t get_length();
};


class GridAxes{
public:
  // all n input dimensions to specify the performance space
  GridAxes();
  GridAxes(std::vector<GridAxis> grid_axes);

  std::vector<GridAxis> axes;

  std::size_t get_ndims();
  std::size_t get_dim_length(std::size_t);
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
  std::vector<double> get_values(std::vector<std::size_t> coords);
  // double get_value(std::size_t table_index, std::vector<std::size_t> coords);

private:
  void construct_axes(std::vector< std::vector<double> > &grid);
  Eigen::ArrayXXd construct_values(
    std::vector< std::vector<double> > &values
  );
  void check_inputs(
    std::vector< std::vector<double> > &grid,
    std::vector< std::vector<double> > &values
  );

  GridAxes grid_axes;
  Eigen::ArrayXXd value_tables;
  std::size_t ndims;
  std::vector<std::size_t> dimension_lengths;
  std::size_t num_tables;
  std::size_t num_values;
};


// free functions
bool free_check_sorted(std::vector<double>);
std::size_t locate_coords(
  std::vector<std::size_t> coords,
  std::vector<std::size_t> dimension_lengths
);
std::vector<double> eigen_to_vector(Eigen::ArrayXd);


}
#endif // GRIDDEDDATA_H_
