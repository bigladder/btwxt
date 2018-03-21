/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDDEDDATA_H_
#define GRIDDEDDATA_H_

#include<vector>

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


class ValueTable{
public:
  // individual vector for a single output variable (e.g., capacity)
  ValueTable();
  ValueTable(
    double *value_array,
    std::size_t size,
    std::vector<std::size_t> &dimension_lengths
  );
  ValueTable(
    std::vector<double> value_vector,
    std::vector<std::size_t> &dimension_lengths
  );

  double get_value(std::vector<std::size_t> coords);

private:
  std::vector<double> values;
  std::vector<std::size_t> dimension_lengths;
};


class AllValueTables{
  // vector of vectors for the complete set of output variables
public:
  AllValueTables();
  AllValueTables(
    std::vector< std::vector<double> > values,
    std::vector<std::size_t> dimension_lengths
  );

  std::vector<ValueTable> value_table_vec;

  std::size_t get_ntables();
};


class GriddedData{
public:
  GriddedData();
  GriddedData(
    std::vector< std::vector<double> > grid,
    std::vector< std::vector<double> > values
  );

  std::size_t get_ndims();
  double get_value(std::size_t table_index, std::vector<std::size_t> coords);

private:
  void construct_axes(std::vector< std::vector<double> > &grid);
  void construct_values(
    std::vector< std::vector<double> > &values,
    std::vector<std::size_t> dimension_lengths
  );
  void check_inputs(
    std::vector< std::vector<double> > &grid,
    std::vector< std::vector<double> > &values
  );

  GridAxes grid_axes;
  AllValueTables all_the_values;
};


// free functions
bool free_check_sorted(std::vector<double>);

}
#endif // GRIDDEDDATA_H_
