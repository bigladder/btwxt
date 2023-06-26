[![Build and Test](https://github.com/bigladder/btwxt/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/bigladder/btwxt/actions/workflows/build-and-test.yml)
[![codecov](https://codecov.io/gh/bigladder/btwxt/branch/master/graph/badge.svg)](https://codecov.io/gh/bigladder/btwxt)

# **btwxt**

## General-purpose, N-dimensional interpolation library...

**btwxt** is a free and open source C++ library to perform multivariate interpolation of one or more functions represented by known values on provided data sets associated with coordinates on a regular (rectilinear) grid. The primary class is a RegularGridInterpolator constructed from:

1. a set of one or more *grid axes*, representing the unique variates for interpolation, each specfied through a set of *grid-axis coordinates*, and
2. a collection of one or more *data sets*, repesenting the function values at each of the *control points* specified by ordered pairs of the grid-axis coordinates.

A RegularGridInterpolator object can then be queried repeatedly for function values within the space of the grid axes. Interpolation is performed for when a grid coordinate is inside the range of a particular grid axis; extrapolation is performed (up to a user-defined limit) when the grid coordinate is outside that range. A general query of a multivariate function may entail any combination of interpolation and extrapolation on the collection of grid axes.

**btwxt** supports various methods for both interpolation and extrpolation; different methods can be specifed for each axis. Interpolation can be performed using either linear or cubic spline. Whereas both linear and cubic splines maintain continuity of a function along the associated axis, cubic splines also maintain continuty of the function's first derivative on that axis. Extrapolation can be either constant or linear, with the option to specify finite extrapolation limits for a given axis.

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
SlopeMethod slope_method = SlopeMethod::quadratic;
double slope_reduction = 0.0;
std::pair<double, double> extrapolation_limits{0, 20};
GridAxis(one_axis, "x", interpolation_method, extrapolation_method, extrapolation_limits,
slope_method, slope_reduction);
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
### Single-Axis Spline Interpolation
An interpolation along a single axis generates a function that spans the intervals between control points, typically corresponding to a linear combination of the data values. A spline is essentially a chain of such functions, such that a different set of coefficients is identified for each interval. For example, in the interval [$x_{k}$, $x_{k+1}$] we write
$$h_k(\mu_k) =  \sum^{n+1}_{i=-n}\sigma_{i}^{(k)} (\mu_k)\cdot f_{k+i}$$
where $f_{k}$ is the data value at $x_{k}$ and
$$\mu_{k}={x-x_{k}\over w_{k}}$$
The interval width is given by $w_{k}=x_{k+1}-x_{k}$. It is useful to limit the range of the summation for practical computational. Without loss of generality, we can focus on the case $k=0$, simplifying the notation:
$$h(\mu) =  \sum^{n+1}_{i=-n}\sigma_{i} (\mu)\cdot f_{i}$$
using $\mu_{0}\rightarrow\mu$ and $w_{0}\rightarrow w$. 

Various orders of differentiability may berequired of the spline. Continuity of the $N^{th}$-order derivative of $h(\mu)$ can be satisfied with polynomials of order $2N+1$, motivating the prominence of linear and cubic splines. 
#### Linear case
A *linear* spline can be generated with $n=0$, such that data values $f_0$ and $f_1$ are reproduced precisely at $x_0$ and $x_1$, respectively:
$$h(\mu)=(1-\mu)\cdot f_{0}+\mu\cdot f_{1}$$
The coefficients are clearly
$$\sigma_{0}(\mu)=1-\mu, \qquad \sigma_{1}(\mu)=\mu$$
The resulting interpolation preserve will consist of straight line segments spanning each interval between control points.

#### Cubic case
A *cubic* interpolation generates a smoother function that can be constructed to have continuous first derivative (slope), which is useful for many types of data analysis. The general form can be written
$$h(\mu)=a_0(\mu)\cdot f_0+a_1(\mu)\cdot f_1+b_0(\mu)\cdot w_0\cdot f'_0++b_1(\mu)\cdot w_0\cdot f'_1$$
with
$$\begin{align*}a_0(\mu)&=(1-\mu)\cdot [1+\mu\cdot (1-2\mu)]\\
a_1(\mu)&=\mu\cdot[1-(1-\mu)\cdot(1-2\mu)]\\
b_0(\mu)&=(1-\mu)^2\cdot \mu\\
b_1(\mu)&=-(1-\mu)\cdot \mu^2\\
\end{align*}$$
Nonetheless, the slope is not uniquely defined for a discrete data set, so a reasonable estimation must be generated by inspection of neighboring control points.
In fact, we need consider only the additional data values at $x_{k-1}$ and $x_{k+1}$ to develop an expression for $f'_{k}$:
$$f'_{k}=(1-\alpha)\cdot [(1-\beta_{k})\cdot r_{k}+\beta_{k}\cdot r_{k-1}]$$
where
$$r_{k}=\frac{f_{k+1}-f_{k}}{w_{k}}$$
The factor $1-\alpha$ uniformly scales the slope at control points. A default value of $\alpha =0$ is assigned. Various criteria for the specification of $\beta _{k}$ may be applicable in different contexts; **btwxt** allows provides options for slope evaluation. The coefficient may depend on the relative widths of neighboring intervals. We define
$$t_{k}=\frac{w_{k}}{w_{k-1}}$$
The default setting computes the slope by a local fit to a *quadratic* function about each control point, which corresponds to
$$\beta_{k}=\frac{t_{k}}{1 + t_{k}}$$
The *cardinal* method is realized with
$$\beta_{k}=\frac{1}{1 + t_{k}}$$
The slope in *finite-difference* methods is computed with $\beta _{k}=0.5$\, imposing no width dependence.

We can now represent the cubic spline over [$x_{0}$, $x_{1}$] as the sum
$$h(\mu)=\sigma_{-1}(\mu)\cdot f_{-1}+\sigma_{0}(\mu)\cdot f_{0}+\sigma_{1}(\mu)\cdot f_{1}+\sigma_{2}(\mu)\cdot f_{2}$$
where
$$\begin{align*}
\sigma_{-1}(\mu)=&b_0(\mu)\cdot S_{-1}^{(-)}\\
\sigma_{0}(\mu)=&a_0(\mu)+b_0(\mu)\cdot S_{0}^{(-)}+b_1(\mu)\cdot S_{0}^{(+)}\\
\sigma_{1}(\mu)=&a_1(\mu)+b_0(\mu)\cdot S_{1}^{(-)}+b_1(\mu)\cdot S_{1}^{(+)}\\
\sigma_{2}(\mu)=&b_1(\mu)\cdot S_{2}^{(+)}\\
\end{align*}$$
The six parameters $S$, referred to as *cubic-slope coefficients*, are found to be
$$\begin{align*}
S_{-1}^{(-)}=&-t_0\cdot \beta_0\\
S_0^{(-)}=&(1+t_0)\cdot \beta_0-1,\qquad
S_{0}^{(+)}=-\beta_1\\
S_1^{(-)}=&1-\beta_0,\qquad
S_{1}^{(+)}=-1/t_1+(1+1/t_1)\cdot\beta_1\\
S_{2}^{(+)}=&(1-\beta_1)/t_1\\
\end{align*}$$
