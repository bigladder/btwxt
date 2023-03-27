/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <chrono>
#include <iostream>

// vendor
#include <fmt/format.h>

// btwxt
#include "public-fixtures.h"

namespace Btwxt {

// return an evenly spaced 1-d vector of doubles.
std::vector<double> linspace(double start, double stop, std::size_t number_of_points)
{
    std::vector<double> result(number_of_points);
    double step = (stop - start) / (static_cast<double>(number_of_points) - 1.);
    double value = start;
    for (std::size_t i = 0; i < number_of_points; i++) {
        result[i] = value;
        value += step;
    }
    return result;
}

TEST_F(FunctionFixture, scipy_3d_grid)
{
    // Based on
    // https://docs.scipy.org/doc/scipy/reference/generated/scipy.interpolate.RegularGridInterpolator.html
    grid.resize(3);
    grid[0] = linspace(1, 4, 11);
    grid[1] = linspace(4, 7, 22);
    grid[2] = linspace(7, 9, 33);

    functions = {[](std::vector<double> x) -> double {
        return 2 * x[0] * x[0] * x[0] + 3 * x[1] * x[1] - x[2];
    }};
    setup();

    const double epsilon = 0.0001;
    double result;
    double expected_value;

    target = {2.1, 6.2, 8.3};
    result = interpolator(target)[0];
    expected_value = 125.80469388; // Interpolated value from example
    EXPECT_NEAR(result, expected_value, epsilon);

    target = {3.3, 5.2, 7.1};
    result = interpolator(target)[0];
    expected_value = 146.30069388; // Interpolated value from example
    EXPECT_NEAR(result, expected_value, epsilon);
}

TEST_F(FunctionFixture, scipy_2d_grid)
{
    // Based on
    // https://docs.scipy.org/doc/scipy/reference/generated/scipy.interpolate.RegularGridInterpolator.html
    grid.resize(2);
    grid[0] = {-2, 0, 4};
    grid[1] = {-2, 0, 2, 5};

    functions = {[](std::vector<double> x) -> double { return x[0] * x[0] + x[1] * x[1]; }};
    setup();
    interpolator.set_axis_extrapolation_method(0, Method::linear);
    interpolator.set_axis_extrapolation_method(1, Method::linear);
    interpolator.set_axis_extrapolation_limits(0, {-5, 10});
    interpolator.set_axis_extrapolation_limits(1, {-5, 10});

    auto test_axis_values1 = linspace(-4, 9, 31);
    auto& test_axis_values2 = test_axis_values1;
    std::vector<std::vector<double>> target_space {test_axis_values1, test_axis_values2};
    auto targets = cartesian_product(target_space);
    for (const auto& t : targets) {
        double result = interpolator(t)[0];
        double expected_value = functions[0](t);

        bool extrapolating = false;

        for (auto target_bounds_axis : interpolator.get_target_bounds_status()) {
            extrapolating |= target_bounds_axis != TargetBoundsStatus::interpolate;
        }
        double epsilon = 7.;
        if (extrapolating) {
            epsilon = 75.; // It's not a very good approximation : )
        }
        EXPECT_NEAR(result, expected_value, epsilon)
            << fmt::format("difference evaluates to {}", std::abs(result - expected_value));
    }
}

TEST_F(GridFixture, four_point_1d_cubic_interpolate)
{

    grid = {{0, 2, 5, 10}};
    data_sets = {{6, 5, 4, 3}};
    target = {2.5};
    setup();

    interpolator.set_axis_interpolation_method(0, Method::cubic);

    const double expected_value = 4.804398;
    const double epsilon = 0.0001;
    double result = interpolator.get_values_at_target(target)[0];
    EXPECT_NEAR(result, expected_value, epsilon);
}

TEST_F(GridFixture, empty_grid_throw_test)
{
    grid = {{}};
    data_sets = {{}};
    target = {};
    EXPECT_THROW(setup(), BtwxtException);
}

TEST_F(GridFixture, single_point_1d_extrapolate)
{
    grid = {{2.}};
    data_sets = {{5.}};
    target = {2.5};
    setup();
    interpolator.set_axis_extrapolation_method(0, Method::cubic);
    double result = interpolator.get_values_at_target(target)[0];
    EXPECT_NEAR(result, 5., 0.0001);
}

TEST_F(GridFixture, two_point_cubic_1d_interpolate)
{
    grid = {{0, 10}};
    data_sets = {{6, 3}};
    target = {2.5};
    setup();
    interpolator.set_axis_interpolation_method(0, Method::cubic);
    double result = interpolator.get_values_at_target(target)[0];
    EXPECT_NEAR(result, 5.25, 0.0001);
}

TEST_F(GridFixture2D, target_undefined)
{
    std::vector<double> returned_target;
    std::string expected_stdout =
        "  WARNING: The current target was requested, but no target has been set.\n";

    // The test fixture does not instantiate a GridPoint.
    EXPECT_STDOUT(returned_target = interpolator.get_target();, expected_stdout)
    std::vector<double> expected_result = {0, 0};
    EXPECT_EQ(returned_target, expected_result);

    double bad_result;
    std::string ResultsExpectedOut =
        "  WARNING: Results were requested, but no target has been set.\n";
    EXPECT_STDOUT(bad_result = interpolator.get_value_at_target(0);, ResultsExpectedOut)
    EXPECT_EQ(bad_result, 0);

    // Define the target; make sure it works now.
    interpolator.set_target(target);
    std::string empty_out; // intentionally default ""
    EXPECT_STDOUT(returned_target = interpolator.get_target();, empty_out)
    expected_result = {12, 5};
    EXPECT_EQ(returned_target, expected_result);

    // Clear the target; see that it reverts to warnings.
    interpolator.clear_target();
    EXPECT_STDOUT(returned_target = interpolator.get_target();, expected_stdout)
    expected_result = {0, 0};
    EXPECT_EQ(returned_target, expected_result);

    EXPECT_STDOUT(bad_result = interpolator.get_value_at_target(0);, ResultsExpectedOut)
    EXPECT_EQ(bad_result, 0);
}

TEST_F(GridFixture2D, interpolate)
{
    interpolator.set_target(target);

    // All values, current target
    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(4.2), testing::DoubleEq(8.4)));
    // Single value, current target
    double d_result = interpolator.get_value_at_target(0);
    EXPECT_DOUBLE_EQ(d_result, 4.2);

    std::vector<double> another_target = {8.1, 4.2};
    // All values, fresh target
    result = interpolator.get_values_at_target(another_target);
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(3.189), testing::DoubleEq(6.378)));
    // Single value, fresh target
    d_result = interpolator.get_value_at_target(another_target, 1);
    EXPECT_DOUBLE_EQ(d_result, 6.378);
}

TEST_F(GridFixture2D, extrapolate)
{
    // axis1 is designated constant extrapolation
    target = {10, 3};
    std::vector<double> result = interpolator(target);
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(2), testing::DoubleEq(4)));

    // axis0 is designated linear extrapolation
    target = {18, 5};
    result = interpolator(target);
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(1.8), testing::DoubleEq(3.6)));
}

TEST_F(GridFixture2D, invalid_inputs)
{
    std::vector<double> short_target = {1};
    std::string expected_error {"  ERROR: Target and grid do not have the same dimensions.\n"};
    // Redirect cout to temporary local buffer (do not use EXPECT_STDOUT for throwing functions)
    std::ostringstream buffer;
    std::streambuf* sbuf = std::cout.rdbuf();
    std::cout.rdbuf(buffer.rdbuf());

    try {
        interpolator.set_target(short_target);
    }
    catch (BtwxtException&) {
        EXPECT_STREQ(expected_error.c_str(), buffer.str().c_str());
    }
    std::vector<double> long_target = {1, 2, 3};
    buffer.str("");
    buffer.clear();
    try {
        interpolator.set_target(long_target);
    }
    catch (BtwxtException&) {
        EXPECT_STREQ(expected_error.c_str(), buffer.str().c_str());
    }
    std::cout.rdbuf(sbuf);

    std::vector<double> data_set = {6, 3, 2, 8, 4};
    EXPECT_THROW(interpolator.add_grid_point_data_set(data_set);, BtwxtException);
    data_set = {1, 1, 1, 1, 1, 1, 1};
    EXPECT_THROW(interpolator.add_grid_point_data_set(data_set);, BtwxtException);
}

TEST_F(GridFixture2D, logger_modify_context)
{
    std::vector<double> returned_target;
    std::string expected_error =
        "  WARNING: The current target was requested, but no target has been set.\n";
    EXPECT_STDOUT(returned_target = interpolator.get_target();, expected_error)
    std::string context_str {"Context 1:"};
    interpolator.get_logger()->set_message_context(reinterpret_cast<void*>(&context_str));
    expected_error =
        "Context 1:  WARNING: The current target was requested, but no target has been set.\n";
    EXPECT_STDOUT(interpolator.get_target();, expected_error)
}

TEST_F(GridFixture2D, unique_logger_per_rgi_instance)
{
    std::vector<double> returned_target;
    std::string expected_error =
        "  WARNING: The current target was requested, but no target has been set.\n";
    EXPECT_STDOUT(returned_target = interpolator.get_target();, expected_error)

    auto logger2 = std::make_shared<BtwxtContextCourierr>();
    std::string context_str {"RGI2 Context:"};
    logger2->set_message_context(reinterpret_cast<void*>(&context_str));
    RegularGridInterpolator rgi2(interpolator, logger2);
    std::string expected_error2 {
        "RGI2 Context:  WARNING: The current target was requested, but no target has been set.\n"};
    EXPECT_STDOUT(rgi2.get_target();, expected_error2)

    EXPECT_STDOUT(interpolator.get_target();, expected_error) // Recheck
}

TEST_F(GridFixture2D, access_logger_in_btwxt)
{
    RegularGridInterpolator rgi2(interpolator, std::make_shared<BtwxtContextCourierr>());
    std::string context_str {"RGI2 Context:"};
    rgi2.get_logger()->set_message_context(reinterpret_cast<void*>(&context_str));
    std::string expected_error2 {
        "RGI2 Context:  WARNING: The current target was requested, but no target has been set.\n"};
    EXPECT_STDOUT(rgi2.get_target();, expected_error2)
}

TEST_F(GridFixture2D, cubic_interpolate)
{
    interpolator.set_axis_interpolation_method(0, Method::cubic);
    interpolator.set_axis_interpolation_method(1, Method::cubic);
    interpolator.set_target(target);

    // All values, current target
    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(4.416), testing::DoubleEq(8.832)));
}

TEST_F(GridFixture2D, normalize)
{
    interpolator.set_axis_interpolation_method(0, Method::cubic);
    interpolator.set_axis_interpolation_method(1, Method::cubic);
    interpolator.set_target(target);

    // All values, current target
    interpolator.normalize_grid_point_data_at_target(
        static_cast<std::size_t>(0u)); // normalize first grid point data set
    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(1.0), testing::DoubleEq(8.832)));
}

TEST_F(GridFixture2D, write_data)
{
    EXPECT_EQ("Axis 1,Axis 2,Data 1,Data 2,\n"
              "0,4,6,12,\n"
              "0,6,3,6,\n"
              "10,4,2,4,\n"
              "10,6,8,16,\n"
              "15,4,4,8,\n"
              "15,6,2,4,\n",
              interpolator.write_data());
}

TEST_F(FunctionFixture2D, normalization_return_scalar)
{
    target = {7.0, 3.0};
    std::vector<double> normalization_target = {2.0, 3.0};
    double expected_divisor {functions[0](normalization_target)};
    double expected_value_at_target {functions[0](target) / expected_divisor};
    double return_scalar =
        interpolator.normalize_grid_point_data_at_target(0, normalization_target, 1.0);
    interpolator.set_target(target);
    std::vector<double> results = interpolator.get_values_at_target();
    EXPECT_THAT(return_scalar, testing::DoubleEq(expected_divisor));
    EXPECT_THAT(results, testing::ElementsAre(expected_value_at_target));
}

TEST_F(FunctionFixture2D, normalization_return_compound_scalar)
{
    target = {7.0, 3.0};
    std::vector<double> normalization_target = {2.0, 3.0};
    double normalization_divisor = 4.0;
    double expected_compound_divisor {functions[0](normalization_target) * normalization_divisor};
    double expected_value_at_target {functions[0](target) / expected_compound_divisor};
    double return_scalar = interpolator.normalize_grid_point_data_at_target(
        0, normalization_target, normalization_divisor);
    interpolator.set_target(target);
    std::vector<double> results = interpolator.get_values_at_target();
    EXPECT_THAT(return_scalar, testing::DoubleEq(expected_compound_divisor));
    EXPECT_THAT(results, testing::ElementsAre(expected_value_at_target));
}

TEST_F(FunctionFixture4D, construct)
{
    interpolator.set_target(target);

    std::vector<double> returned_target = interpolator.get_target();
    EXPECT_THAT(returned_target, testing::ElementsAre(2.2, 3.3, 1.4, 4.1));
}

TEST_F(FunctionFixture4D, calculate)
{
    interpolator.set_target(target);

    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_NEAR(result[0], functions[0](target), 0.02);
    EXPECT_DOUBLE_EQ(result[1], functions[1](target));
}

TEST_F(FunctionFixture4D, verify_linear)
{
    // no matter what we do, result[1] should always be 11!
    std::vector<double> result;

    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(0, Method::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(3, Method::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(0, Method::linear);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(2, Method::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(0, Method::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(1, Method::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);
}

TEST_F(FunctionFixture4D, timer)
{
    interpolator.set_target(target);

    // Get starting time point
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<double> result = interpolator.get_values_at_target();
    // Get ending time point
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    interpolator.get_logger()->info(
        fmt::format("Time taken by interpolation: {} microseconds", duration.count()));

    // time running the functions straight
    start = std::chrono::high_resolution_clock::now();
    // double r0 = fn0(target[0], target[1], target[2], target[3]);
    // double r1 = fn1(target[0], target[1], target[2], target[3]);
    // Get ending time point
    stop = std::chrono::high_resolution_clock::now();
    auto nano_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    interpolator.get_logger()->info(
        fmt::format("Time taken by direct functions: {} nanoseconds", nano_duration.count()));
}

TEST_F(FunctionFixture4D, multi_timer)
{
    std::vector<std::vector<double>> set_of_targets = {{0.1, 0.1, 0.1, 0.1},
                                                       {3.3, 2.2, 4.1, 1.4},
                                                       {2.1, 1.6, 1.6, 2.1},
                                                       {3.7, 4.3, 0.8, 2.1},
                                                       {1.9, 3.4, 1.2, 1.1},
                                                       {3.3, 3.8, 1.6, 3.0},
                                                       {0.3, 1.0, 2.4, 1.1},
                                                       {3.1, 1.9, 2.9, 3.3},
                                                       {4.2, 2.7, 1.3, 4.4},
                                                       {2.1, 2.9, 1.8, 1.9}};

    for (std::size_t count = 0; count < 10; count++) {
        // Get starting time point
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& target : set_of_targets) {
            std::vector<double> result = interpolator(target);
        }
        // Get ending time point
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
        interpolator.get_logger()->info(
            fmt::format("Time taken by ten interpolations: {} microseconds", duration.count()));
    }
}

TEST(CartesianProduct, cartesian_product)
{
    std::vector<std::vector<short>> v = {{1, 2, 3}, {4, 5}, {6, 7, 8, 9}};
    std::vector<std::vector<short>> result = cartesian_product(v);
    EXPECT_EQ(result.size(), 3u * 2u * 4u);
    EXPECT_THAT(result[0], testing::ElementsAre(1, 4, 6));
    EXPECT_THAT(result[1], testing::ElementsAre(1, 4, 7));
    EXPECT_THAT(result[10], testing::ElementsAre(2, 4, 8));
    EXPECT_THAT(result[3 * 2 * 4 - 1], testing::ElementsAre(3, 5, 9));
}

} // namespace Btwxt