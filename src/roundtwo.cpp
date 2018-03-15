/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include<vector>

//btwxt
#include "roundtwo.h"

namespace Btwxt {



WhereInTheGridIsThisPoint::WhereInTheGridIsThisPoint() {};



GridPoint::GridPoint() {};
GridPoint::GridPoint(double* target) {};
GridPoint::GridPoint(std::vector<double> target_vector) :
  size(target_vector.size()),
  target(new double[size])
{
  std::copy(target_vector.begin(), target_vector.end(), target);
  std::cout << "GridPoint object constructed from vector!" << std::endl;
};



GridAxis::GridAxis() {};
GridAxis::GridAxis(double* grid, std::size_t size) {};
GridAxis::GridAxis(std::vector<double> grid_vector) :
  size(grid_vector.size()),
  grid(new double[size])
{
  std::copy(grid_vector.begin(), grid_vector.end(), grid);
  std::cout << "GridAxis object constructed from vector!" << std::endl;
};

// GridAxis::~GridAxis()
// {
//   delete[] grid;
// };



GridAxes::GridAxes() {};
GridAxes::GridAxes(std::size_t ndims, std::vector<GridAxis> grid_axes):
  ndims(ndims),
  axes(grid_axes)
{
  std::vector<std::size_t> dim_lengths;
  for (auto a : axes) {
    dim_lengths.push_back(a.size);
  }
  std::cout << ndims << "-D GridAxis object constructed!" << std::endl;
  bool first_item = true;
  for (auto d: dim_lengths)
  {
      std::cout << (first_item ? "dimension lengths = {" : ", ") << d;
      first_item = false;
  }
  std::cout << "}" << std::endl;
};



ValueTable::ValueTable() {};
ValueTable::ValueTable(double *value_array, std::size_t size) {};
ValueTable::ValueTable(std::vector<double> value_vector) :
  size(value_vector.size()),
  values(new double[size])
{
  std::copy(value_vector.begin(), value_vector.end(), values);
  std::cout << "ValueTable object constructed from vector!" << std::endl;
};

double ValueTable::get_value(std::vector<std::size_t> coords) {return 0.0; };
double ValueTable::evaluate_linear(GridPoint target_point) {return 0.0; };



AllValueTables::AllValueTables() {};
AllValueTables::AllValueTables(std::vector< std::vector<double> >& values)
{
  num_tables = values.size();
  for (auto v : values) {
    ValueTable vt(v);
    value_table_vec.push_back(vt);
  }
  std::cout << "Vector of ValueTables created comprising " << num_tables << " tables!" << std::endl;
};


RegularGridInterpolator::RegularGridInterpolator() {};

RegularGridInterpolator::RegularGridInterpolator(
  std::vector< std::vector<double> > grid,
  std::vector< std::vector<double> > values
)
{
  ndims = grid.size();
  std::vector<GridAxis> axis_vector;
  for (auto axis : grid) {
    GridAxis ga(axis);
    axis_vector.push_back(ga);
  }
  GridAxes grid_axes(ndims, axis_vector);
  AllValueTables all_the_values(values);
};

double RegularGridInterpolator::calculate_value_at_target(std::vector<double> target, ValueTable* this_output)
{return 0.0; };
double RegularGridInterpolator::calculate_value_at_target(ValueTable* this_output)
{return 0.0; };
std::vector<double> RegularGridInterpolator::calculate_all_values_at_target(std::vector<double> target)
{
  std::vector<double> result = {0.0};
  return result;
};
std::vector<double> RegularGridInterpolator::calculate_all_values_at_target()
{
  std::vector<double> result = {0.0};
  return result;
};

void RegularGridInterpolator::set_new_grid_point(std::vector<double> target)
{
  current_grid_point = GridPoint(target);
};

std::vector<size_t> RegularGridInterpolator::get_coordinates_of_current_grid_point()
{
  std::vector<size_t> result = {0};
  return result;
};
void RegularGridInterpolator::clear_current_grid_point() {};
std::size_t RegularGridInterpolator::get_ndims()
{ return ndims; };
void RegularGridInterpolator::do_common_constructor_things() {};

}
