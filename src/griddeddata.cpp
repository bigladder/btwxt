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




GriddedData::GriddedData() {};
GriddedData::GriddedData(
  std::vector< std::vector<double> > grid,
  std::vector< std::vector<double> > values
)
{
  dimension_lengths = check_inputs(grid, values);
  construct_axes(grid);
  value_tables = construct_values(values);
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

  ndims = grid_axes.get_ndims();
  showMessage(MSG_INFO, std::to_string(ndims) + "-D GridAxis object constructed");
};


Eigen::ArrayXXd GriddedData::construct_values(
  std::vector< std::vector<double> > &values
)
{
  num_tables = values.size();
  num_values = 1;
  for (auto a : dimension_lengths) {
    num_values *= a;
  }
  showMessage(MSG_INFO, "We expect " + std::to_string(num_values) + " values.");
  Eigen::ArrayXXd vtables(num_tables, num_values);
  showMessage(MSG_INFO, "Created blank Eigen Array with " + std::to_string(vtables.rows()) + " tables, each with " + std::to_string(vtables.cols())+ " values.");
  std::size_t i = 0;
  for (auto v : values) {
    Eigen::Map< Eigen::ArrayXd > temp_row(&v[0], num_values);
    vtables.row(i) = temp_row;
    i++;
  }

  std::cout << vtables << std::endl;
  // TODO: I would prefer to be using the class value_tables over returning an eigen array
  return vtables;
};

std::vector<std::size_t> GriddedData::check_inputs(
  std::vector< std::vector<double> > &grid,
  std::vector< std::vector<double> > &values
)
{
  // check that grid dimensions and value tables align
  std::size_t expected_nvalues = 1;
  std::vector<std::size_t> dimension_lengths;
  for (auto grid_vector : grid) {
    expected_nvalues *= grid_vector.size();
    dimension_lengths.push_back(grid_vector.size());
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
  return dimension_lengths;
};

std::size_t GriddedData::get_ndims()
{ return grid_axes.get_ndims(); };

std::size_t GriddedData::get_num_tables()
{ return num_tables; };

std::vector<double> GriddedData::get_values(std::vector<std::size_t> coords)
{
  std::size_t index = locate_coords(coords, dimension_lengths);
  if (index == -1) {return {0};} ;
  // return column of values as a vector
  double* col_data = value_tables.col(index).data();
  std::vector<double> one_column(col_data, col_data+num_tables);
  return one_column;
}

Eigen::ArrayXd GriddedData::get_column(std::vector<std::size_t> coords)
{
  std::size_t index = locate_coords(coords, dimension_lengths);
  // TODO handle exception if invalid coordinates
  // if (index == -1) { return {0}; } ;
  return value_tables.col(index);
}

std::vector<double> GriddedData::get_grid_vector(std::size_t grid_index)
{
  return grid_axes.axes[grid_index].grid;
}


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


std::size_t locate_coords(
  std::vector<std::size_t> coords,
  std::vector<std::size_t> dimension_lengths
)
{
  std::size_t index = 0;
  std::size_t prev_len = 1;
  for (std::size_t d=0; d<dimension_lengths.size(); d++)
  {
    if (coords[d] >= dimension_lengths[d]) {
      showMessage(MSG_WARN, "You overran dimension " + std::to_string(d));
      return -1;
    }
    else {
      index += coords[d] * prev_len;
      prev_len = dimension_lengths[d];
    }
  }
  // showMessage(MSG_INFO, "The unrolled index is " + std::to_string(index));
  return index;
}

std::vector<double> eigen_to_vector(Eigen::ArrayXd change_this)
{
  std::size_t length = change_this.rows();
  double* c_array = change_this.data();
  std::vector<double> to_this(c_array, c_array+length);
  return to_this;
}

}
