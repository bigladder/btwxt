/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include<vector>

//btwxt
#include "roundtwo.h"
#include "error.h"

namespace Btwxt {



WhereInTheGridIsThisPoint::WhereInTheGridIsThisPoint() {};



GridPoint::GridPoint() {};
GridPoint::GridPoint(double* target) {};
GridPoint::GridPoint(std::vector<double> target_vector) :
  size(target_vector.size()),
  target(new double[size])
{
  std::copy(target_vector.begin(), target_vector.end(), target);
  showMessage(MSG_INFO, "GridPoint object constructed from vector!");
};



GridAxis::GridAxis() {};
GridAxis::GridAxis(double* grid, std::size_t size) {};
GridAxis::GridAxis(std::vector<double> grid_vector) :
  size(grid_vector.size()),
  grid(new double[size])
{
  std::copy(grid_vector.begin(), grid_vector.end(), grid);
  showMessage(MSG_INFO, "GridAxis object constructed from vector!");
  bool grid_is_sorted = check_sorted();
  if (grid_is_sorted) {
    showMessage(MSG_INFO, "axis is sorted.");
  }
  else {
    showMessage(MSG_ERR, "axis is not sorted.");
  }
};

// GridAxis::~GridAxis()
// {
//   delete[] grid;
// };

bool GridAxis::check_sorted() {
  // this allows either ascending or descending order
  bool is_asc = std::is_sorted(grid, (grid+size) );
  // bool is_desc = std::is_sorted(grid, grid+size, lesser);
  return is_asc;
};


GridAxes::GridAxes() {};
GridAxes::GridAxes(std::size_t ndims, std::vector<GridAxis> grid_axes):
  ndims(ndims),
  axes(grid_axes)
{
  std::vector<std::size_t> dim_lengths;
  for (auto a : axes) {
    dim_lengths.push_back(a.size);
  }
  showMessage(MSG_INFO, std::to_string(ndims) + "-D GridAxis object constructed");
  bool first_item = true;
  std::string message_str = "dimension lenths = ";
  for (auto d: dim_lengths)
  {
      message_str += (first_item ? "{" : ", ");
      first_item = false;
      message_str += std::to_string(d);
  }
  message_str += "}";
  showMessage(MSG_INFO, message_str);
};



ValueTable::ValueTable() {};
ValueTable::ValueTable(double *value_array, std::size_t size) {};
ValueTable::ValueTable(std::vector<double> value_vector) :
  size(value_vector.size()),
  values(new double[size])
{
  std::copy(value_vector.begin(), value_vector.end(), values);
  showMessage(MSG_INFO, "ValueTable object constructed from vector!");
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
  std::string message_str = "Vector of ValueTables created comprising "
          + std::to_string(num_tables) + " tables!";
  showMessage(MSG_INFO, message_str);
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

  // check that grid dimensions and value tables align
  std::size_t expected_nvalues = 1;
  for (auto a : grid_axes.axes) {
    expected_nvalues *= a.size;
  }
  // for (auto d : grid_axes.dim_lengths) {
  //   expected_nvalues *= d;  //  <-- why doesn't this work?
  // }
  std::string message_str = "We expect " + std::to_string(expected_nvalues)
          + " values in each table.";
  showMessage(MSG_INFO, message_str);
  // TODO fix Callback, uncomment.
  for (auto value_vector : values) {
    std::string message_str = "Value vector has "
            + std::to_string(value_vector.size()) + " values.";
    showMessage(MSG_INFO, message_str);
    if (value_vector.size() != expected_nvalues) {
      showMessage(MSG_ERR, "Input value table does not match the grid size");
    }
  }
  AllValueTables all_the_values(values);
  GridPoint current_grid_point({-999});
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
  if (ndims == target.size()) {
    showMessage(MSG_INFO, "Target and GridAxes dimensions match: " + std::to_string(target.size()));
  }
  else {
    std::string message_str = "number of dimensions (" + std::to_string(ndims)
            + ") does not match length of target (" + std::to_string(target.size()) + ").";
    showMessage(MSG_ERR, message_str);
  }
  current_grid_point = GridPoint(target);
};

std::vector<double> RegularGridInterpolator::get_coordinates_of_current_grid_point()
{
  std::vector<double> result;
  result.assign(current_grid_point.target, current_grid_point.target+ndims);
  return result;
};

void RegularGridInterpolator::clear_current_grid_point() {};
std::size_t RegularGridInterpolator::get_ndims()
{ return ndims; };
void RegularGridInterpolator::do_common_constructor_things() {};

}
