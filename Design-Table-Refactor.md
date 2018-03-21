# Refactoring of Performance Tables

## Btwxt Overview

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
* Floor - along one GridAxis, the index of the grid-defined point less-than-or-equal-to a specified GridPoint. For example, for `GridAxis = {5, 10, 15, 20}`, `Floor(12, GridAxis) = 1`. (1 being the index of the 10 value in the array.)
* Weight - the fraction defined by how far between the floor and the next point on a GridAxis a GridPoint falls. Referenced from the floor. For example, for `GridAxis = {5, 10, 15, 20}`, `Weight(12, GridAxis) = 0.4`.


## API Overview

The GriddedData constructor is being designed to work smoothly with the PerformanceData FlatBuffer file format being defined for ASHRAE Section 205. It will also be easy to instantiate a GriddedData object from a well-defined set of input vectors and value arrays.

Each GridAxis object will be able to have the following attributes defined independently:
* extrapolation method
* extrapolation limits
* interpolation method (future release)

With `RegularGridInterpolator my_interpolator` defined by `GriddedData my_gridded_data` and `GridPoint target`, these are the ways to perform an interpolation:
```
std::size_t table_index;  // to define the ValueTable of interest
double result = my_interpolator.calculate_value_at_target(target, table_index);
std::vector<double> result_vector = my_interpolator.calculate_all_values_at_target(target);
```

The library has overloaded the "()" operator, so the following are also available:
```
double result = my_interpolator(target, table_index);
std::vector<double> result_vector = my_interpolator(target);
```

The library will also store a `current_grid_point`, enabling these methods:
```
double result = my_interpolator.calculate_value_at_target(table_index);
double result = my_interpolator(table_index);
std::vector<double> result_vector = my_interpolator. calculate_all_values_at_target();
std::vector<double> result_vector = my_interpolator();
```

Accordingly, the `RegularGridInterpolator` API will have methods to set, get, and clear `current_grid_point`.


## Architecture Overview

`class GridPoint` has attributes:
* `target` to describe the axis coordinates.
* a vector of tuples `floors_and_weights`, used in the interpolation.

`class GriddedData` holds both a `ValueTables` and a `GridSpace` object.
* has validation methods to check that the size of the value arrays match up with the dimensionality of the grid.
* has a method to return the ValuePoint at a point defined by grid indices (i.e., by index rather than by value).

`class RegularGridInterpolator` wraps `GriddedData` and holds a `GridPoint current_grid_point`.


## Algorithmic Overview

The linear interpolation algorithm included in Btwxt is an iterative hypercube interpolator. The steps:

1. collect the 2^N ValuePoints that describe the N-Dimensional hypercube surrounding the target.
2. Set n=N.
3. Pair ValuePoints to form 2^(n-1) edges in that dimension
4. Perform the 2^(n-1) simple linear interpolations for those pairs: `x = x_0 * (1-weight) + x_1 * weight` for x in (x_0, x_1).
5. Considering the 2^(n-1) reesulting ValuePoints a new hypercube of dimension N-1, decrement n and return to step 3.
6. Stop when the hypercube is collapsed to a single ValuePoint. Return that tuple of values.


Later task to add speedup to the interpolation by reducing dimensionality when an input is fixed.

## Performance Benchmarking

The Btwxt library will include a CPU/memory performance benchmark mechanism.
Six-variable function, ten grid steps on each axis.
Select 1000 GridPoints in the defined region. Interpolate and time.
