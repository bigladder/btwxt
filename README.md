[![Build and Test](https://github.com/bigladder/btwxt/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/bigladder/btwxt/actions/workflows/build-and-test.yml)
[![codecov](https://codecov.io/gh/bigladder/btwxt/branch/master/graph/badge.svg)](https://codecov.io/gh/bigladder/btwxt)

# **btwxt**

## General-purpose, N-dimensional interpolation library...

**btwxt** is a free and open source C++ library to perform multivariate interpolation of one or more functions represented by known values on provided data sets associated with coordinates on a regular (rectilinear) grid. The primary class is a RegularGridInterpolator constructed from:

1. a set of one or more *grid axes*, representing the unique variates for interpolation, each specfied through a set of *grid-axis coordinates*, and
2. a collection of one or more *data sets*, repesenting the function values at each of the *control points* specified by ordered pairs of the grid-axis coordinates.

A RegularGridInterpolator object can then be queried repeatedly for function values within the space of the grid axes. Interpolation is performed for when a grid coordinate is inside the range of a particular grid axis; extrapolation is performed (up to a user-defined limit) when the grid coordinate is outside that range. A general query of a multivariate function may entail any combination of interpolation and extrapolation on the collection of grid axes.

**btwxt** supports various methods for both interpolation and extrpolation; different methods can be specifed for each axis. Interpolation can be performed using either linear or cubic (Catmull-Rom) spline. Whereas both linear and cubic splines maintain continuity of a function along the associated axis, cubic splines also maintain continuty of the function's first derivative on that axis. Extrapolation can be either constant or linear, with the option to specify finite extrapolation limits for a given axis.

The grid is required to be rectilinear, but the axes are not required to have regular (uniform) spacing. However, the grid coordinates provided for each axis must be strictly increasing.

Data sets are imported as a single long vector (i.e., a flattened array). The user must ensure that values in a data set are ordered, with respect to the orders of both grid coordinates and grid axes, such that each value is correctly associated with the appropriate control point. **btwxt** verifies that the number of values with each data set corresponds with the total number of control points.

## How to Use

### API

The snippet below illustrates instantiation of an individual axis.

```c++
std::vector<double> one_axis{6, 10, 15};
GridAxis(one_axis, "x");  // Construct a new axis named "x"
// interpolation method defaults to linear
// extrapolation method defaults to constant
// extrapolation limits default to {-DBL_MAX, DBL_MAX}
Method extrapolation_method = Method::linear;
Method interpolation_method = Method::cubic;
std::pair<double, double> extrapolation_limits{0, 20};
GridAxis(one_axis, "x", interpolation_method, extrapolation_method, extrapolation_limits);
```

A RegularGridInterpolator holds a collection of axes, which specify the control points, and a collection of data sets containing function values for each of the control points. The snippet below illustrates instantiation of a two-dimensional grid, an associated data set, and their assocation with a RegularGridInterpolator.

```c++
std::vector<std::vector<double>> vector_grid = {{6, 10, 15},
                                                 {4, 6, 9}};
std::vector<std::vector<double>> values = {2, 4, 7, 1, 3, 6};
RegularGridInterpolator my_interpolator(vector_grid, values);
```

Once a RegularGridInterpolator has been synthesized, the interpolated can be queried for function values at target grid-axis coordinates. Typical syntax to perform such a query on a 2-D grid is shown in the snippet below. 

```c++
// 1. set the target before requesting values:
std::vector<double> target{12.5, 5.1};
my_interpolator.set_target(target);
std::vector<double> result = my_interpolator.get_values_at_target();

// 2. set the target when requesting values:
std::vector<double> target{12.5, 5.1};
std::vector<double> result = my_interpolator.get_values_at_target(target);

// 3. set the target by applying the () operator:
std::vector<double> result = my_interpolator({12.5, 5.1});
```
### Single-Axis Interpolation
#### Linear case
A *linear* interpolation over the interval [$x_{k}$, $x_{k+1}$] can be expressed in terms of the data values $f_{k}$ and $f_{k+1}$ as
$$h_{k}(\mu_{k}) =  (1-\mu)\cdot f_{k}+\mu \cdot f_{k+1}$$

where
$$\mu_{k} =  {x-x_{k}\over w_{k}}$$

using the interval width $w_{k}=x_{k+1}-x_{k}$. The resulting interpolation will consist of straight line segments spanning each interval between control points.

#### Cubic case
A *cubic* interpolation generates a smoother funciton that can be constructed to have continuous first derivative (slope), which is useful for many types of data analysis. We apply the formulation used above to a pair of functions $g_{k}(\mu_{k})$ and $g_{k+1}(\mu_{k})$, up to quadratic in order, associated with each control point, rather than the data values, themselves. The interpolation becomes
$$h(\mu_{k}) =  (1-\mu_{k})\cdot g_{k}(\mu_{k})+\mu_{k} \cdot g_{k+1}(\mu_{k})$$

(Note that the representations of these functions may depend on the argument indicated.) To enforce continuity, these two functions must intersect and have the appropriate data value at both $x_{k}$ and $x_{k+1}$. Although the slope is not uniquely defined for a discrete data set, it can be estimated by inspection of neighboring control points. It is useful to express the slope at $x_{k}$ as
$$f'_{k}=(1-c_{k})\cdot r_{k}+c_{k}\cdot r_{k-1}$$
where
$$r_{k}=\frac{f_{k+1}-f_{k}}{w_{k}}$$
In **btwxt**, the user can select from among various methods to specify $c_{k}$, which may be applicable in different contexts.