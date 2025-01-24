import btwxt
import pytest
from itertools import product

def test_grid_axis():
    axis_x = btwxt.GridAxis()
    assert axis_x.get_length() == 0

    xs = [30, 35, 40, 45, 50, 55]
    axis_x = btwxt.GridAxis(
        [30, 35, 40, 45, 50, 55], # Values
        btwxt.InterpolationMethod_cubic, # interpolation method
        btwxt.ExtrapolationMethod_linear, # extrapolation_method
        (20.0, 60.0), # extrapolation_limits
        "LWT", # name
    )
    assert axis_x.get_length() == 6
    assert set(axis_x.get_values()) == set(xs)
    assert axis_x.get_interpolation_method() == btwxt.InterpolationMethod_cubic
    assert axis_x.get_extrapolation_method() == btwxt.ExtrapolationMethod_linear
    assert axis_x.get_extrapolation_limits() == (20.0, 60.0)


def F(x, y):
    return x**2 + y**2

def test_interpolator():
    x0 = [1.0, 2.0, 3.0]
    x1 = [10.0, 20.0]
    # Not using numpy on purpose
    ys = [F(x,y) for (x, y) in product(x0, x1)]

    axis_x0 = btwxt.GridAxis(
        x0, # Values
        btwxt.InterpolationMethod_cubic, # interpolation method
        btwxt.ExtrapolationMethod_linear, # extrapolation_method
        (0, 5), # extrapolation_limits
        "x0", # name
    )
    axis_x1 = btwxt.GridAxis(
        x1, # Values
        btwxt.InterpolationMethod_cubic, # interpolation method
        btwxt.ExtrapolationMethod_linear, # extrapolation_method
        (0, 30), # extrapolation_limits
        "x1", # name
    )
    grid_point_y = btwxt.GridPointDataSet(ys, "y")

    interpolator = btwxt.RegularGridInterpolator([axis_x0, axis_x1], [grid_point_y])

    target_normalization = [2, 10]
    normalizationDivisor = 1.0
    normalizationDivisor = interpolator.normalize_grid_point_data_set_at_target(
        0, target_normalization, normalizationDivisor
    ) * normalizationDivisor
    assert normalizationDivisor == 104.0

    interp_x0s = [float(x) for x in range(0, 6, 1)]
    interp_oat_dbs = [float(x) for x in range(0, 31, 5)]

    test_points = [(x,y) for (x, y) in product(interp_x0s, interp_oat_dbs)]
    true_values = [F(x, y) for (x, y) in test_points]
    interp_values = [
        interpolator.get_value_at_target((x0, x1), 0) * normalizationDivisor
        for (x0, x1) in test_points
    ]
