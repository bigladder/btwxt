/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>
#include<vector>

// btwxt
#include "griddeddata.h"
#include "error.h"

namespace Btwxt {


GridAxis::GridAxis() {};
GridAxis::GridAxis(double* grid, std::size_t size) {};
GridAxis::GridAxis(std::vector<double> grid_vector) :
  grid(grid_vector)
{
  showMessage(MSG_INFO, "GridAxis object constructed from vector!");

  bool grid_is_sorted = Btwxt::free_check_sorted(grid);
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

std::size_t GridAxis::get_length()
{ return grid.size(); };


GridAxes::GridAxes() {};
GridAxes::GridAxes(std::vector<GridAxis> grid_axes) : axes(grid_axes) {};

std::size_t GridAxes::get_ndims()
{
  // showMessage(MSG_INFO, "Asking for ndims: " + std::to_string(axes.size()));
  return axes.size();
};

std::size_t GridAxes::get_dim_length(std::size_t dim)
{
  std::size_t ndims = axes.size();
  if (dim >= ndims) {
    showMessage(MSG_WARN, "We don't have that many dimensions.");
    return 0;
  }
  else {
    return axes[0].get_length();
  }
}

std::vector<std::size_t> GridAxes::get_dim_lengths()
{
  std::vector<std::size_t> dim_lengths;
  for (auto grid : axes) {
    dim_lengths.push_back(grid.get_length());
  }
  return dim_lengths;
}


ValueTable::ValueTable() {};
ValueTable::ValueTable(
  double *value_array,
  std::size_t size,
  std::vector<std::size_t> &dimension_lengths
) {};

ValueTable::ValueTable(
  std::vector<double> value_vector,
  std::vector<std::size_t> &dimension_lengths
) :
  values(value_vector),
  dimension_lengths(dimension_lengths)
{
  showMessage(MSG_INFO, "ValueTable object constructed from vector!");
};

double ValueTable::get_value(
  std::vector<std::size_t> coords)
{
  std::size_t index = 0;
  std::size_t prev_len = 1;
  for (std::size_t d=0; d<dimension_lengths.size(); d++)
  {
    if (coords[d] >= dimension_lengths[d]) {
      showMessage(MSG_WARN, "You overran dimension " + std::to_string(d));
      return 0;
    }
    else {
      index += coords[d] * prev_len;
      prev_len = dimension_lengths[d];
    }
  }
  showMessage(MSG_INFO, "The unrolled index is " + std::to_string(index));
  return values[index];
};




AllValueTables::AllValueTables() {};
AllValueTables::AllValueTables(
  std::vector< std::vector<double> > values,
  std::vector<std::size_t> dimension_lengths
)
{
  for (auto v : values) {
    ValueTable vt(v, dimension_lengths);
    value_table_vec.push_back(vt);
  }

  std::size_t num_tables = values.size();
  std::string message_str = "Vector of ValueTables created comprising "
          + std::to_string(num_tables) + " tables!";
  showMessage(MSG_INFO, message_str);
};

std::size_t AllValueTables::get_ntables()
{ return value_table_vec.size(); }



GriddedData::GriddedData() {};
GriddedData::GriddedData(
  std::vector< std::vector<double> > grid,
  std::vector< std::vector<double> > values
)
{
  check_inputs(grid, values);
  construct_axes(grid);
  construct_values(values, grid_axes.get_dim_lengths());
  showMessage(MSG_INFO, "GriddedData constructed from vectors!");
};

void GriddedData::construct_axes(
  std::vector< std::vector<double> > &grid
)
{
  for (auto axis : grid) {
    GridAxis ga(axis);
    grid_axes.axes.push_back(ga);
  }

  std::size_t ndims = grid_axes.get_ndims();
  showMessage(MSG_INFO, std::to_string(ndims) + "-D GridAxis object constructed");
};

void GriddedData::construct_values(
  std::vector< std::vector<double> > &values,
  std::vector<std::size_t> dimension_lengths
)
{
  for (auto v : values) {
    ValueTable vt(v, dimension_lengths);
    all_the_values.value_table_vec.push_back(vt);
  }

  std::size_t num_tables = values.size();
  std::string message_str = "Vector of ValueTables created comprising "
          + std::to_string(num_tables) + " tables!";
  showMessage(MSG_INFO, message_str);
};

void GriddedData::check_inputs(
  std::vector< std::vector<double> > &grid,
  std::vector< std::vector<double> > &values
)
{
  // check that grid dimensions and value tables align
  std::size_t expected_nvalues = 1;
  for (auto a : grid) {
    expected_nvalues *= a.size();
  }
  std::string message_str = "We expect " + std::to_string(expected_nvalues)
          + " values in each table.";
  showMessage(MSG_INFO, message_str);

  for (auto value_vector : values) {
    std::string message_str = "Value vector has "
            + std::to_string(value_vector.size()) + " values.";
    showMessage(MSG_INFO, message_str);
    if (value_vector.size() != expected_nvalues) {
      showMessage(MSG_ERR, "Input value table does not match the grid size");
    }
  }
};

std::size_t GriddedData::get_ndims()
{ return grid_axes.get_ndims(); };

double GriddedData::get_value(
  std::size_t table_index,
  std::vector<std::size_t> coords
)
{
  if (table_index >= get_ndims()) {
    showMessage(MSG_WARN, "We don't have that many tables.");
    return 0;
  }
  else {
    return all_the_values.value_table_vec[table_index].get_value(coords);
  }
};


// free functions
bool free_check_sorted(std::vector<double> my_vec)
{
  std::vector<double>::iterator first = my_vec.begin();
  std::vector<double>::iterator last = my_vec.end();
  bool is_asc = true;
  bool is_desc = true;
  if (first==last) return true;

  std::vector<double>::iterator next = first;
  while (++next!=last) {
    if (is_asc & (*next<=*first))
      is_asc = false;
    if (is_desc & (*next>=*first))
      is_desc = false;
    if (!is_asc & !is_desc)
      return false;
    ++first;
  }
  return true;
};

}
