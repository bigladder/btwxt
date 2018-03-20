/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDINTERP_H_
#define GRIDINTERP_H_

#include<vector>

// We will eventually split this out into two header files,
//     the public API and the private members.


namespace Btwxt{

class WhereInTheGridIsThisPoint{
public:
  WhereInTheGridIsThisPoint();
  std::size_t floor;
  double weight;
};


class GridPoint{
public:
  // target is an array of doubles specifying the point we are interpolating to.
  GridPoint();
  GridPoint(double* target);
  GridPoint(std::vector<double> &target_vector);

  std::vector<double> target;
  std::vector<WhereInTheGridIsThisPoint> floors_and_weights;
  bool is_inbounds;  // for deciding interpolation vs. extrapolation;
};


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
private:
  bool check_sorted();
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



// this will be the public-facing class.
class RegularGridInterpolator{
public:
  // GridAxes, GridAxis, AllValueTables, ValueTable are instantiated in RGI constructor.
  RegularGridInterpolator();
  RegularGridInterpolator(GriddedData &the_blob);
  RegularGridInterpolator(
    std::vector< std::vector<double> > grid,
    std::vector< std::vector<double> > values
  );
  // RegularGridInterpolator(
  //   std::vector< std::vector<double> > grid,
  //   double values[], std::size_t size);
  // RegularGridInterpolator(
  //   std::vector< double[] > grid, std::vector<std::size_t> grid_dimensions,
  //   double values[], std::size_t size);  // we may not want to allow this constructor. feels dangerous.

  // GridPoint gets instantiated inside calculate_value_at_target
  double calculate_value_at_target(std::vector<double> target, ValueTable* this_output);
  double operator()(std::vector<double> target, ValueTable* this_output)
  {
    return calculate_value_at_target(target, this_output);
  }

  double calculate_value_at_target(ValueTable* this_output);
  double operator()(ValueTable* this_output)
  {
    return calculate_value_at_target(this_output);
  }

  std::vector<double> calculate_all_values_at_target(std::vector<double> target);
  std::vector<double> operator()(std::vector<double> target)
  {
    return calculate_all_values_at_target(target);
  }

  std::vector<double> calculate_all_values_at_target();
  std::vector<double> operator()()
  {
    return calculate_all_values_at_target();
  }

  void set_new_grid_point(std::vector<double> target);
  std::vector<double> get_current_grid_point();
  void clear_current_grid_point();
  std::size_t get_ndims();

private:
  GriddedData the_blob;
  GridPoint current_grid_point;

  void check_target_dimensions(std::vector<double> target);
};

}
#endif // GRIDINTERP_H_
