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
  // Formerly known as PerformancePoint
  // target is an array of doubles specifying the point we are interpolating to.
  GridPoint();
  GridPoint(double* target);
  double* target;
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

  std::size_t size;
  double *grid;
  // bool is_regular;  <-- to add later
private:
  // only called as GridAxis.find_floor_and_weight from within GridAxes.
  // std::pair<std::size_t, double> find_floor_and_weight(double target);
};


class GridAxes{
public:
  // all n input dimensions to specify the performance space
  GridAxes();
  GridAxes(std::size_t ndims, std::vector<GridAxis>);

  std::size_t ndims;
  std::vector<GridAxis> axes;
  std::vector<std::size_t> dim_lengths;

  // void find_floor_and_weight(GridPoint target_point);
  // std::vector< std::pair<std::size_t, double> > find_floors_and_weights(double target[]);
};


class ValueTable{
public:
  // individual flat table for a single output variable (e.g., capacity)
  ValueTable();
  ValueTable(double *value_array, std::size_t size);
  ValueTable(std::vector<double> value_vector);

  std::size_t size;
  std::vector<std::size_t> dimension_lengths;
  double* values;
  double get_value(std::vector<std::size_t> coords);
  double evaluate_linear(GridPoint target_point);
  // collect_hypercube
};


class AllValueTables{
  // pointers to a set of flat tables for the complete set of output variables
public:
  AllValueTables();
  AllValueTables(std::vector< std::vector<double> >& values);
  std::size_t num_tables;
  // TODO: use unique_ptr<ValueTable> instead of ValueTable.
  std::vector<ValueTable> value_table_vec;
  // std::vector<std::unique_ptr<ValueTable> > value_table_ptrs;
};


// this will be the public-facing class.
class RegularGridInterpolator{
public:
  // GridAxes, GridAxis, AllValueTables, ValueTable are instantiated in RGI constructor.
  RegularGridInterpolator();
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
  std::vector<size_t> get_coordinates_of_current_grid_point();
  void clear_current_grid_point();
  std::size_t get_ndims();

private:
  std::size_t ndims;
  GridAxes grid_axes;
  AllValueTables all_the_values;
  GridPoint current_grid_point;

  void do_common_constructor_things();
};

}
#endif // GRIDINTERP_H_
