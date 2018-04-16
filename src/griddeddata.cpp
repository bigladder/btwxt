/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */


// Standard
#include<iostream>

// btwxt
#include "griddeddata.h"
#include "error.h"

namespace Btwxt {


GridAxis::GridAxis() {};
GridAxis::GridAxis(double* grid, std::size_t size,
  int extrapolation_method, int interpolation_method) {};
GridAxis::GridAxis(std::vector<double> grid_vector,
  int extrapolation_method, int interpolation_method) :
  grid(grid_vector),
  extrapolation_method(extrapolation_method),
  interpolation_method(interpolation_method)
{
  if (interpolation_method == CUBIC) {
    spacing_multiplier = calc_spacing_multipliers();
  }

  showMessage(MSG_DEBUG, "GridAxis object constructed from vector!");

  bool grid_is_sorted = Btwxt::free_check_sorted(grid);
  if (grid_is_sorted) {
    showMessage(MSG_DEBUG, "axis is sorted.");
  }
  else {
    showMessage(MSG_ERR, "axis is not sorted.");
  }
};

std::size_t GridAxis::get_length() {
  return grid.size();
}

void GridAxis::set_interp_method(const int im) {
  interpolation_method = im;
  if (im==CUBIC) {
    spacing_multiplier = calc_spacing_multipliers();
  }
}

double GridAxis::get_spacing_multiplier(const std::size_t& flavor,
  const std::size_t& index)
{
  return spacing_multiplier[flavor][index];
}

std::vector< std::vector<double> > GridAxis::calc_spacing_multipliers() {
  std::size_t grid_size = grid.size();
  std::vector< std::vector<double> > v(2, std::vector<double>(grid_size-1, 1.0));

  double center_spacing;
  for (std::size_t i=0; i<grid.size()-1; i++) {
    center_spacing = grid[i+1]-grid[i];
    if (i!=0) {
      v[0][i] = center_spacing/(grid[i+1]-grid[i-1]);
    }
    if (i+2!=grid_size) {
      v[1][i] = center_spacing/(grid[i+2]-grid[i]);
    }
  }
  return v;
}





GriddedData::GriddedData() {};
GriddedData::GriddedData(
  std::vector< std::vector<double> > grid,
  std::vector< std::vector<double> > values
) :
  // TODO move interp_methods down to GridAxis class
  interp_methods(grid.size(), LINEAR)
{
  ndims = grid.size();
  num_values = 1;
  for (auto grid_vector : grid) {
    num_values *= grid_vector.size();
    dimension_lengths.push_back(grid_vector.size());
  }
  num_tables = values.size();

  construct_axes(grid);
  value_tables = construct_values(values);
  showMessage(MSG_DEBUG, "GriddedData constructed from vectors!");
};

void GriddedData::construct_axes(
  const std::vector< std::vector<double> >& grid
)
{
  for (auto axis : grid) {
    GridAxis ga(axis);
    grid_axes.push_back(ga);
  }

  showMessage(MSG_DEBUG, stringify(ndims, "-D GridAxis object constructed"));
};


Eigen::ArrayXXd GriddedData::construct_values(
  const std::vector< std::vector<double> >& values
)
{
  Eigen::ArrayXXd vtables(num_tables, num_values);
  // #ifdef TESTLOG
  showMessage(MSG_DEBUG, stringify("Created blank Eigen Array with ",
      vtables.rows(), " tables, each with ", vtables.cols(), " values."));
  showMessage(MSG_DEBUG, stringify("We expect ", num_values, " values in each table."));
  // #endif //TESTLOG
  std::size_t i = 0;
  for (auto value_vector : values) {

    if (value_vector.size() != num_values) {
      showMessage(MSG_ERR, stringify(
        "Input value table does not match the grid size: ",
        value_vector.size(), " != ", num_values));
    }

    Eigen::Map< Eigen::ArrayXd > temp_row(&value_vector[0], num_values);
    vtables.row(i) = temp_row;
    i++;
  }
  showMessage(MSG_DEBUG, stringify("value tables: \n", vtables));
  // TODO: I would prefer to be using the class value_tables over returning an eigen array
  return vtables;
};


std::size_t GriddedData::get_ndims()
{ return grid_axes.size();; };

std::size_t GriddedData::get_num_tables()
{ return num_tables; };

std::vector<double> GriddedData::get_values(
  const std::vector<std::size_t>& coords)
{
  std::size_t index = locate_coords(coords, dimension_lengths);
  if (index == -1) {return {0};} ;
  // return column of values as a vector
  double* col_data = value_tables.col(index).data();
  std::vector<double> one_column(col_data, col_data+num_tables);
  return one_column;
}

Eigen::ArrayXd GriddedData::get_column(
  const std::vector<std::size_t>& coords)
{
  std::size_t index = locate_coords(coords, dimension_lengths);
  // TODO handle exception if invalid coordinates
  // if (index == -1) { return {0}; } ;
  return value_tables.col(index);
}

Eigen::ArrayXd GriddedData::get_column_near(
  std::vector<std::size_t> coords, const std::size_t& dim, const int& i)
{
  coords[dim] += i;
  return get_column(coords);
}

Eigen::ArrayXd GriddedData::get_column_near(
  std::vector<std::size_t> coords, const std::vector<int>& translation)
{
  // coords.size() must equal translation.size()
  std::transform(coords.begin( ), coords.end( ),
                 translation.begin( ), coords.begin( ),
                 std::plus<int>());
  return get_column(coords);
}

std::vector<double> GriddedData::get_grid_vector(const std::size_t& grid_index)
{
  return grid_axes[grid_index].grid;
}

double GriddedData::get_axis_spacing_mult(const std::size_t& grid_index,
  const std::size_t& flavor, const std::size_t& index) {
    if (interp_methods[grid_index] == CUBIC) {
      return grid_axes[grid_index].get_spacing_multiplier(flavor, index);
    } else {
      return 0.0;
    }
}

void GriddedData::set_axis_extrap_method(
  const std::size_t& grid_index, const int extrapolation_method)
{
  grid_axes[grid_index].extrapolation_method = extrapolation_method;
}

std::vector<int> GriddedData::get_extrap_methods()
{
  std::vector<int> extrap_methods(ndims);
  for (std::size_t dim=0; dim<ndims; dim++) {
    extrap_methods[dim] = grid_axes[dim].extrapolation_method;
  }
  return extrap_methods;
}

std::vector<int> GriddedData::get_interp_methods()
{ return interp_methods; }

void GriddedData::set_axis_interp_method(
  const std::size_t& grid_index, const int interpolation_method)
{
  interp_methods[grid_index] = interpolation_method;
  grid_axes[grid_index].set_interp_method(interpolation_method);
}

// free functions
// TODO this function allows ascending or descending, which may be too generous.
// reconsider at a later date.
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
  const std::vector<std::size_t>& coords,
  const std::vector<std::size_t>& dimension_lengths
)
{
  std::size_t index = 0;
  std::size_t panel_size = 1;
  for (std::size_t d=0; d<dimension_lengths.size(); d++)
  {
    if (coords[d] >= dimension_lengths[d]) {
      showMessage(MSG_WARN, stringify("You overran dimension ", d));
      return -1;
    }
    else {
      index += coords[d] * panel_size;
      panel_size *= dimension_lengths[d];
    }
  }
  // showMessage(MSG_DEBUG, stringify("The unrolled index is ", index));
  return index;
}

std::vector<double> eigen_to_vector(Eigen::ArrayXd& change_this)
{
  std::size_t length = change_this.rows();
  double* c_array = change_this.data();
  std::vector<double> to_this(c_array, c_array+length);
  return to_this;
}

}
