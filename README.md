[![Build and Test](https://github.com/bigladder/btwxt/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/bigladder/btwxt/actions/workflows/build-and-test.yml)
[![codecov](https://codecov.io/gh/bigladder/btwxt/branch/master/graph/badge.svg)](https://codecov.io/gh/bigladder/btwxt)

# Btwxt

## General-purpose, N-dimensional interpolation library...

Btwxt is a free and open source C++ library to perform numerical interpolation on a regular grid data set. The primary
class is a RegularGridInterpolator constructed from:

1. a set of N grid axes representing the values of the independent variables, and
2. a collection of one or more grid point data sets aligned to the grid axes.

A RegularGridInterpolator object can then be queried repeatedly for interpolated values that
fall inside its grid points, or extrapolated values beyond the grid points (up to a limit defined by the user).

Btwxt supports linear and cubic spline (Catmull-Rom) interpolation methods. Both linear and cubic splines maintain continuity of values; cubic splines also maintain continuty of the first derivatives. Different methods can be specified for each 
axis. The API also allows specification of preferred extrapolation methods (constant or linear)--again independently for
each axis--and extrapolation limits.

The grid is required to be regular (rectilinear), but the axes are not required to have uniform spacing. Each axis need
only be strictly increasing.

The grid point data sets are imported as a single long vector (i.e., a flattened array). It is the user's responsibility
to ensure that the grid point data is ordered according to the order of grid axes. Btwxt ensures that the number of 
grid points aligns with the total combined points of the grid axes.

## How to Use

### API

An individual axis can be instantiated with:

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

A RegularGridInterpolator holds a collection of axes and the corresponding grid point data at all permutations of the grid axes' values. 

```c++
std::vector<std::vector<double>> vector_grid = {{6, 10, 15},
                                                 {4, 6, 9}};
std::vector<std::vector<double>> values = {...};
RegularGridInterpolator my_interpolator(vector_grid, values);
```

Once you have an RegularGridInterpolator object, you can set a point to interpolate to:

```c++
std::vector<double> target{12.5, 5.1};
my_interpolator.set_target(target);
std::vector<double> result = my_interpolator.get_values_at_target();

// or set the target when requesting values:
std::vector<double> new_target{11.7, 6.1};
result = my_interpolator.get_values_at_target(new_target);
```

We have overloaded the () operator on the RegularGridInterpolator to perform the interpolation, enabling the following:

```c++
std::vector<double> target{12.5, 5.1};
my_interpolator.set_target(target);
std::vector<double> result = my_interpolator();
std::vector<double> new_target{11.7, 6.1};
result = my_interpolator(new_target);
```
### Cubic Interpolation
Cubic interpolation within an interval between points $P_{0}$, and $P_{1}$ can be performed with consideration of the adjacent points, $P_{-1}$, and $P_{2}$, exterior to the interval in either direction. We define a dimensionless variable
$$\mu =  {x-x_{0}\over x_{1}-x_{0}}$$
The three points $P_{-1}$, $P_{0}$, and $P_{1}$ uniquely specify a quadractic curve $g_{0}(\mu)$; $P_{0}$, $P_{1}$, and $P_{2 }$ specify another quadractic curve $g_{1}(\mu)$. Note that these two functions both contain the points $P_{0}$ and $P_{1}$ that define our interval of interest, so the linear interpolation
$$h(\mu) =  (1-\mu)\cdot g_{0}(\mu)+\mu \cdot g_{1}(\mu)$$
 will also contain these points. Further analysis reveals that the first derivatives at these points are equal to those of $g_{0}$ and $g_{1}$, respectively. Continuity of both the value and first derivative are therefore maintained.