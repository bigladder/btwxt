# Refactoring of Performance Tables

## Btwxt Overview

This document describes the planned design of the interpolation library, Task #1 in the statement of work. Design for tasks #2 and #3---which relate to refactoring  EnergyPlus `table` procedures---will be described in a separate document.

The proposal is to write a general-purpose N-dimensional interpolation library, Btwxt, for gridded data. After reviewing existing open-source libraries, we believe it is sensible to write a new library.

The library will accept:
* a collection of N vectors representing the input variables
* an array of (or collection of arrays of) values that map onto the grid defined by the input vectors

Together, the input vectors and array(s) of output values will define a performance space. The library will allow querying of a defined performance space for a well-defined point within the grid.

* A point fully inside the grid boundaries will receive an interpolated value.
 * The initial interpolation method will be linear.
 * The library structure will allow for alternative interpolation methods to be added later.
* A point outside the grid boundaries will receive an extrapolated value.
 * The library will support (a) linear and (b) constant extrapolation
 * The library will allow each input dimension to define its extrapolation method and an extrapolation limit.

## Glossary

* *GridAxis* - The collection of values that define points along a single grid dimension (often thought of as an independent variable)
* *GridSpace* - The collection of multiple GridAxis objects that fully define the performance space.
* *ValueTables* - A collection of result/output values that all map to the same GridSpace.
* *GriddedData* - A unification of a GridSpace with its associated ValueTables.
* *GridPoint* - A coordinate set (one value along each axis) that defines a single point within a GridSpace.
* *ValuePoint* - the set of result/output values at a single coordinate location. For example, the output values for a GridPoint.
* *Floor* - along one GridAxis, the index of the grid-defined point less-than-or-equal-to a specified GridPoint. For example, for `GridAxis = {5, 10, 15, 20}`, `Floor(12, GridAxis) = 1`. (1 being the index of the 10 value in the array.)
* *Weight* - the fraction defined by how far between the floor and the next point on a GridAxis a GridPoint falls. Referenced from the floor. For example, for `GridAxis = {5, 10, 15, 20}`, `Weight(12, GridAxis) = 0.4`.


## Approach

The GriddedData constructor is being designed to work smoothly with the PerformanceData FlatBuffer file format being defined for ASHRAE Section 205. It will also be easy to instantiate a GriddedData object from a well-defined set of input vectors and value arrays.

#### Summaries of important classes:
**GridAxis**

```
class GridAxis{
  GridAxis(std::vector<double> grid_vector);

  // attributes
  std::vector<double> grid;
  int extrap_method;
  std::pair<double, double> extrap_limits;
  // int interp_method;   <-- to add later
  // bool is_regular;     <-- to add later

  // methods
  std::size_t get_length();
  void set_extrap_method();
  void set_extrap_limits();
  // void set_interp_method();  <-- to add later
}
```
The `GridAxis` constructor ensures that the `grid_vector` is sorted either strictly ascending or strictly descending.

The idea behind a future GridAxis-specific interpolation method is to allow the developer to:
- specify an interpolation method for all dimensions at the interpolation level
- set a different interpolation method for a particular dimension to override the general method in that dimension.

The idea behind a future `is_regular` is that finding floors and weights for a target double can be performed algebraically if a grid axis is regularly spaced.

**ValueTables**
```
class ValueTables{
  ValueTables(
    std::vector< std::vector<double> > value_vector,
    std::vector<std::size_t> &dimension_lengths
  );

  // attributes
  std::vector< ValuePoint > values;
  std::vector<std::size_t> dimension_lengths;

  // methods
  std::vector<double> get_values(std::vector<std::size_t> coords);
}
```
*[Commentary on ValueTables]*

**GriddedData**
```
class GriddedData{
  GriddedData(
    std::vector< std::vector<double> > grid,
    std::vector< std::vector<double> > values
  );

  // attributes
  GridAxes grid_axes;
  AllValueTables all_the_values;

  // methods
  void check_inputs();
  double get_values(std::vector<std::size_t> coords);
}
```
`GriddedData` holds both a `ValueTables` and a `GridSpace` object. It has validation methods to check that the size of the value arrays match up with the dimensionality of the grid.

The `GriddedData.get_values()` method wraps the `ValueTables.get_values()` method and makes it available to the public. The method returns the ValuePoint at a point defined by grid indices (i.e., by index rather than by value).

**GridPoint**
```
class GridPoint{
  GridPoint(std::vector<double> &target_vector);

  // attributes
  std::vector<double> target;
  std::vector<std::size_t> floors;
  std::vector<double> weights;
  std::vector<bool> is_inbounds;

  // methods
  void get_floors_and_weights();
}
```

- `target` describes the axis coordinates by value.
- `floors` and `weights` are computed and stored to help the interpolation.
- `is_inbounds` is used to decide interpolation or extrapolation for each GridAxis.


**RegularGridInterpolator**
```
class RegularGridInterpolator{
public:
  RegularGridInterpolator(GriddedData &the_blob);
  RegularGridInterpolator(
    std::vector< std::vector<double> > grid,
    std::vector< std::vector<double> > values
  );

  double calculate_value_at_target(std::vector<double> target, std::size_t table_index);
  double operator()(std::vector<double> target, std::size_t table_index)
  {
    return calculate_value_at_target(target, table_index);
  }

  double calculate_value_at_target(std::size_t table_index);
  double operator()(std::size_t table_index)
  {
    return calculate_value_at_target(table_index);
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
  std::tuple<double> evaluate_linear();

```

Note that the "()" operator is overloaded, allowing a function-like shortcut to interpolation. `my_interpolator.calculate_all_values_at_target(target)` can be shortened to `my_interpolator(target)`.

The library will store a `current_grid_point`, meaning that `my_interpolator()` implies interpolating to that saved `GridPoint`. The floors and weights are saved as attributes of `current_grid_point`, so reusing a point will save those search and algebraic steps.

Accordingly, the `RegularGridInterpolator` API has methods to set, get, and clear `current_grid_point`. If the developer attempts to interpolate to `current_grid_point` before one has been specified, the library will return a warning.

the private `evaluate_linear` method is wrapped by the various public `calculate...` methods and houses the interpolation algorithm itself. Alternative interpolation algorithms would be parallel to the `evaluate_linear` method.


## Algorithmic Overview

The linear interpolation algorithm included in Btwxt is an iterative hypercube interpolator. The steps:

1. Collect the 2<sup>N</sup> ValuePoints that describe the N-Dimensional hypercube surrounding the target.
2. Set n=N.
3. Pair ValuePoints to form the 2<sup>n-1</sup> edges in that dimension
4. Perform the 2<sup>n-1</sup> simple linear interpolations for those pairs: x = x<sub>0</sub> &times; (1-weight) + x<sub>1</sub> &times; weight for x in (x<sub>0</sub>, x<sub>1</sub>).
5. Considering the 2<sup>n-1</sup> resulting ValuePoints a new hypercube of dimension N-1, decrement n and return to step 3.
6. Stop when n=0: the hypercube will be collapsed to a single ValuePoint.


We propose to, in future work, do some caching and preprocessing when an input is static to---in effect---reduce the dimensionality of the starting hypercube.

## Performance Benchmarking

The Btwxt library will include a CPU/memory performance benchmark mechanism.
Six-variable function, ten grid steps on each axis.
Select 1000 GridPoints in the defined region. Interpolate and time.
