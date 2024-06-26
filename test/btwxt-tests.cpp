/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <chrono>
#include <iostream>

// vendor
#include <fmt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// btwxt
#include <btwxt/btwxt.h>
#include "fixtures/public-fixtures.h"

namespace Btwxt {

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
    interpolator.set_axis_extrapolation_method(0, ExtrapolationMethod::linear);
    interpolator.set_axis_extrapolation_method(1, ExtrapolationMethod::linear);
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

TEST(Constructors, test_all_constructors)
{

    // grid_axis_vectors, no grid point data
    RegularGridInterpolator rgi1(std::vector<std::vector<double>> {{1}});

    // grid_axes, no grid point data
    RegularGridInterpolator rgi2({GridAxis({1})});

    // grid_axes, grid_point_data_vectors
    RegularGridInterpolator rgi3({GridAxis({1})}, std::vector<std::vector<double>> {{1}});

    // grid_axis_vectors, grid point data
    RegularGridInterpolator rgi4(std::vector<std::vector<double>> {{1}}, {GridPointDataSet({1})});
}

TEST_F(GridFixture, four_point_1d_cubic_interpolate)
{

    grid = {{0, 2, 5, 10}};
    data_sets = {{6, 5, 4, 3}};
    target = {2.5};
    setup();

    interpolator.set_axis_interpolation_method(0, InterpolationMethod::cubic);

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
    EXPECT_THROW(setup(), std::runtime_error);
}

TEST_F(GridFixture, single_point_1d_extrapolate)
{
    grid = {{2.}};
    data_sets = {{5.}};
    target = {2.5};
    setup();
    interpolator.set_axis_extrapolation_method(0, ExtrapolationMethod::linear);
    double result = interpolator.get_values_at_target(target)[0];
    EXPECT_NEAR(result, 5., 0.0001);
}

TEST_F(GridFixture, grid_axis_error)
{
    grid = {{1., 2.}};
    data_sets = {{5., 5.}};
    target = {2.5};
    setup();
    std::string expected_stdout =
        "  [ERROR] RegularGridInterpolator 'Test RGI': GridAxis 'Axis 1': Upper extrapolation "
        "limit (1.5) is within the range of grid axis values [1, 2].\n";
    EXPECT_STDOUT(
        EXPECT_THROW(interpolator.set_axis_extrapolation_limits(0, {0.5, 1.5}), std::runtime_error);
        , expected_stdout)
}

TEST_F(GridFixture, two_point_cubic_1d_interpolate)
{
    grid = {{0, 10}};
    data_sets = {{6, 3}};
    target = {2.5};
    setup();
    interpolator.set_axis_interpolation_method(0, InterpolationMethod::cubic);
    double result = interpolator.get_values_at_target(target)[0];
    EXPECT_NEAR(result, 5.25, 0.0001);
}

TEST_F(GridFixture, get_neighboring_indices)
{
    grid = {{0, 1, 2}, {0, 1, 2}};

    // data_set[i] = i useful for testing
    // clang-format off
    data_sets = {{
    //  0  1  2 < dim 2
        0, 1, 2, // 0 dim 1
        3, 4, 5, // 1  "
        6, 7, 8  // 2  "
    }};
    // clang-format on
    setup();

    // Outside grid points
    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({-1, -1}), testing::ElementsAre(0));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({-1, 0.5}),
                testing::ElementsAre(0, 1));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({-1, 3}), testing::ElementsAre(2));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({3, 3}), testing::ElementsAre(8));

    // On outside boundaries
    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({0, 0.5}),
                testing::ElementsAre(0, 1));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({0.5, 0}),
                testing::ElementsAre(0, 3));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({2, 1.5}),
                testing::ElementsAre(7, 8));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({0.5, 2}),
                testing::ElementsAre(2, 5));

    // On inside boundaries
    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({1, 0.5}),
                testing::ElementsAre(3, 4));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({0.5, 1}),
                testing::ElementsAre(1, 4));

    // Inside cells
    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({0.5, 0.5}),
                testing::ElementsAre(0, 1, 3, 4));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({0.5, 1.5}),
                testing::ElementsAre(1, 2, 4, 5));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({1.5, 0.5}),
                testing::ElementsAre(3, 4, 6, 7));

    EXPECT_THAT(interpolator.get_neighboring_indices_at_target({1.5, 1.5}),
                testing::ElementsAre(4, 5, 7, 8));

    // On grid points
    for (auto g0 : grid[0]) {
        for (auto g1 : grid[1]) {
            interpolator.set_target({g0, g1});
            EXPECT_THAT(interpolator.get_neighboring_indices_at_target(),
                        testing::ElementsAre(interpolator.get_values_at_target()[0]));
        }
    }
}

TEST_F(Grid2DFixture, target_undefined)
{
    std::vector<double> returned_target;
    std::string expected_stdout = "  [ERROR] RegularGridInterpolator 'Test RGI': The current "
                                  "target was requested, but no target has been set.\n";

    // The test fixture does not instantiate a GridPoint.
    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_target(), std::runtime_error);, expected_stdout)

    std::string ResultsExpectedOut = "  [ERROR] RegularGridInterpolator 'Test RGI': Results were "
                                     "requested, but no target has been set.\n";
    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_value_at_target(0), std::runtime_error);
                  , ResultsExpectedOut)

    // Define the target; make sure it works now.
    interpolator.set_target(target);
    std::string empty_out; // intentionally default ""
    EXPECT_STDOUT(returned_target = interpolator.get_target();, empty_out)
    std::vector<double> expected_result {12, 5};
    EXPECT_EQ(returned_target, expected_result);

    // Clear the target; see that it reverts to errors.
    interpolator.clear_target();
    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_target(), std::runtime_error);, expected_stdout)

    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_value_at_target(0), std::runtime_error);
                  , ResultsExpectedOut)
}

TEST_F(Grid2DFixture, interpolate)
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

TEST_F(Grid2DFixture, extrapolate)
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

TEST_F(Grid2DFixture, invalid_inputs)
{
    constexpr std::string_view expected_error_format {
        "  [ERROR] RegularGridInterpolator 'Test RGI': Target (size={}) and grid (size={}) do not "
        "have the same dimensions.\n"};

    std::vector<double> short_target = {1};
    EXPECT_STDOUT(EXPECT_THROW(interpolator.set_target(short_target), std::runtime_error);
                  , fmt::format(expected_error_format, 1, 2))

    std::vector<double> long_target = {1, 2, 3};
    EXPECT_STDOUT(EXPECT_THROW(interpolator.set_target(long_target), std::runtime_error);
                  , fmt::format(expected_error_format, 3, 2))

    constexpr std::string_view expected_error_format_grid_data {
        "  [ERROR] RegularGridInterpolator 'Test RGI': GridPointDataSet 'Data Set 2': Size ({}) "
        "does not match number of grid points (6).\n"};

    std::vector<double> data_set_too_short = {6, 3, 2, 8, 4};

    EXPECT_STDOUT(
        EXPECT_THROW(interpolator.add_grid_point_data_set(data_set_too_short);, std::runtime_error);
        , fmt::format(expected_error_format_grid_data, 5))

    std::vector<double> data_set_too_long = {1, 1, 1, 1, 1, 1, 1};
    EXPECT_STDOUT(
        EXPECT_THROW(interpolator.add_grid_point_data_set(data_set_too_long);, std::runtime_error);
        , fmt::format(expected_error_format_grid_data, 7))
}

TEST_F(Grid2DFixture, courier_modify_context)
{
    std::string expected_error = "  [ERROR] RegularGridInterpolator 'Test RGI': The current target "
                                 "was requested, but no target has been set.\n";
    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_target(), std::runtime_error);, expected_error)
    std::dynamic_pointer_cast<CourierWithContext>(interpolator.get_courier())->context =
        "Context 1";
    expected_error = "  [ERROR] Context 1: RegularGridInterpolator 'Test RGI': The current target "
                     "was requested, but no target has been set.\n";
    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_target(), std::runtime_error);, expected_error)
}

TEST_F(Grid2DFixture, unique_courier_per_rgi_instance)
{
    std::vector<double> returned_target;
    std::string expected_error = "  [ERROR] RegularGridInterpolator 'Test RGI': The current target "
                                 "was requested, but no target has been set.\n";
    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_target(), std::runtime_error);, expected_error)

    auto courier2 = std::make_shared<CourierWithContext>();
    courier2->context = "RGI2 Context";
    RegularGridInterpolator rgi2(interpolator, courier2);
    std::string expected_error2 {"  [ERROR] RGI2 Context: RegularGridInterpolator 'Test RGI': The "
                                 "current target was requested, but no target has been set.\n"};
    EXPECT_STDOUT(EXPECT_THROW(rgi2.get_target(), std::runtime_error);, expected_error2)

    EXPECT_STDOUT(EXPECT_THROW(interpolator.get_target(), std::runtime_error);
                  , expected_error) // recheck
}

TEST_F(Grid2DFixture, access_courier)
{
    RegularGridInterpolator rgi2(interpolator);
    rgi2.set_courier(std::make_shared<CourierWithContext>());
    std::dynamic_pointer_cast<CourierWithContext>(rgi2.get_courier())->context = "RGI2 Context";
    std::string expected_error2 {"  [ERROR] RGI2 Context: RegularGridInterpolator 'Test RGI': The "
                                 "current target was requested, but no target has been set.\n"};
    EXPECT_STDOUT(EXPECT_THROW(rgi2.get_target(), std::runtime_error);, expected_error2)
}

TEST_F(Grid2DFixture, alternative_courier)
{
    class NewCourier : public BtwxtDefaultCourier {
        void receive_error(const std::string& message) override
        {
            write_message("UH-OH!", message);
            throw std::runtime_error(message);
        }
        void receive_warning(const std::string& message) override
        {
            write_message("UMMM...", message);
        }
        void receive_info(const std::string& message) override { write_message("HEY!", message); }
        void receive_debug(const std::string& message) override { write_message("YUCK!", message); }
        void write_message(const std::string& message_type, const std::string& message) override
        {
            std::cout << fmt::format("  [{}] {}", message_type, message) << std::endl;
        }
    };

    auto new_courier = std::make_shared<NewCourier>();
    RegularGridInterpolator rgi2(interpolator, new_courier);
    std::string expected_error {"  [UH-OH!] RegularGridInterpolator 'Test RGI': The current target "
                                "was requested, but no target has been set.\n"};
    EXPECT_STDOUT(EXPECT_THROW(rgi2.get_target(), std::runtime_error);, expected_error)
}

TEST_F(Grid2DFixture, cubic_interpolate)
{
    interpolator.set_axis_interpolation_method(0, InterpolationMethod::cubic);
    interpolator.set_axis_interpolation_method(1, InterpolationMethod::cubic);
    interpolator.set_target(target);

    // All values, current target
    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(4.416), testing::DoubleEq(8.832)));
}

TEST_F(Grid2DFixture, normalize)
{
    interpolator.set_axis_interpolation_method(0, InterpolationMethod::cubic);
    interpolator.set_axis_interpolation_method(1, InterpolationMethod::cubic);

    // All values, current target
    interpolator.normalize_grid_point_data_sets_at_target(
        target); // normalize first grid point data set
    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(1.0), testing::DoubleEq(1.0)));
}

TEST_F(Function2DFixture, normalization_return_scalar)
{
    target = {7.0, 3.0};
    std::vector<double> normalization_target = {2.0, 3.0};
    double expected_divisor {functions[0](normalization_target)};
    double expected_value_at_target {functions[0](target) / expected_divisor};
    double return_scalar =
        interpolator.normalize_grid_point_data_set_at_target(0, normalization_target, 1.0);
    interpolator.set_target(target);
    std::vector<double> results = interpolator.get_values_at_target();
    EXPECT_THAT(return_scalar, testing::DoubleEq(expected_divisor));
    EXPECT_THAT(results, testing::ElementsAre(expected_value_at_target));
}

TEST_F(Function2DFixture, normalization_return_compound_scalar)
{
    target = {7.0, 3.0};
    std::vector<double> normalization_target = {2.0, 3.0};
    double normalization_divisor = 4.0;
    double expected_compound_divisor {functions[0](normalization_target) * normalization_divisor};
    double expected_value_at_target {functions[0](target) / expected_compound_divisor};
    double return_scalar = interpolator.normalize_grid_point_data_set_at_target(
        0, normalization_target, normalization_divisor);
    interpolator.set_target(target);
    std::vector<double> results = interpolator.get_values_at_target();
    EXPECT_THAT(return_scalar, testing::DoubleEq(expected_compound_divisor));
    EXPECT_THAT(results, testing::ElementsAre(expected_value_at_target));
}

TEST(SimpleData, normalize_after_adding_grid_point_data_set)
{
    std::vector<std::vector<double>> grid {{0., 1.}, {0., 1.}};
    std::vector<std::vector<double>> data_sets {{0.0, 1.75, 1.75, 3.5}, {89., 89., 89., 89.}};
    RegularGridInterpolator interpolator(grid);
    std::size_t data_set_index {0};
    interpolator.add_grid_point_data_set(data_sets[data_set_index]);
    EXPECT_EQ(interpolator.get_number_of_grid_point_data_sets(), 1);
    interpolator.normalize_grid_point_data_set_at_target(data_set_index, {0.5, 0.5}, 1.0);
    data_set_index++;
    interpolator.add_grid_point_data_set(data_sets[data_set_index]);
    EXPECT_EQ(interpolator.get_number_of_grid_point_data_sets(), 2);
    interpolator.normalize_grid_point_data_set_at_target(data_set_index, {0.5, 0.5}, 1.0);
    auto results = interpolator.get_values_at_target({1., 1.});
    EXPECT_NEAR(results[0], 2.0, 0.00001);
    EXPECT_NEAR(results[1], 1.0, 0.00001);

    // Test other getters
    EXPECT_EQ(interpolator.get_number_of_dimensions(), 2);
    EXPECT_EQ(interpolator.get_number_of_grid_points(),
              interpolator.get_grid_axis(0).get_length() *
                  interpolator.get_grid_axis(1).get_length());
    EXPECT_EQ(interpolator.get_number_of_grid_points(),
              interpolator.get_grid_point_data_set(0).data.size());
    EXPECT_EQ(interpolator.get_number_of_grid_points(),
              interpolator.get_grid_point_data_set(1).data.size());
}

TEST_F(Function4DFixture, construct)
{
    interpolator.set_target(target);

    std::vector<double> returned_target = interpolator.get_target();
    EXPECT_THAT(returned_target, testing::ElementsAre(2.2, 3.3, 1.4, 4.1));
}

TEST_F(Function4DFixture, calculate)
{
    interpolator.set_target(target);

    std::vector<double> result = interpolator.get_values_at_target();
    EXPECT_NEAR(result[0], functions[0](target), 0.02);
    EXPECT_DOUBLE_EQ(result[1], functions[1](target));
}

TEST_F(Function4DFixture, verify_linear)
{
    // no matter what we do, result[1] should always be 11!
    std::vector<double> result;

    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(0, InterpolationMethod::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(3, InterpolationMethod::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(0, InterpolationMethod::linear);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(2, InterpolationMethod::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(0, InterpolationMethod::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);

    interpolator.set_axis_interpolation_method(1, InterpolationMethod::cubic);
    interpolator.set_target(target);
    result = interpolator.get_values_at_target();
    EXPECT_DOUBLE_EQ(result[1], 11);
}

TEST_F(Function4DFixture, timer)
{
    interpolator.set_target(target);

    // Get starting time point
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<double> result = interpolator.get_values_at_target();
    // Get ending time point
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    interpolator.get_courier()->send_info(
        fmt::format("Time taken by interpolation: {} microseconds", duration.count()));

    // time running the functions straight
    start = std::chrono::high_resolution_clock::now();
    // double r0 = fn0(target[0], target[1], target[2], target[3]);
    // double r1 = fn1(target[0], target[1], target[2], target[3]);
    // Get ending time point
    stop = std::chrono::high_resolution_clock::now();
    auto nano_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    interpolator.get_courier()->send_info(
        fmt::format("Time taken by direct functions: {} nanoseconds", nano_duration.count()));
}

TEST_F(Function4DFixture, multi_timer)
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
        interpolator.get_courier()->send_info(
            fmt::format("Time taken by ten interpolations: {} microseconds", duration.count()));
    }
}

TEST_F(Grid2DFixture, write_data)
{
    EXPECT_EQ("Axis 1,Axis 2,Data Set 1,Data Set 2,\n"
              "0,4,6,12,\n"
              "0,6,3,6,\n"
              "10,4,2,4,\n"
              "10,6,8,16,\n"
              "15,4,4,8,\n"
              "15,6,2,4,\n",
              interpolator.write_data());
}

TEST(GridPointDataSet, wrong_size)
{
    std::string expected_error {
        "  [ERROR] RegularGridInterpolator 'Unnamed RegularGridInterpolator': GridPointDataSet '': "
        "Size (2) does not match number of grid points (1).\n"
        "  Generated using BtwxtDefaultCourier. Consider deriving your own Courier class!\n"};

    EXPECT_STDOUT(
        EXPECT_THROW(RegularGridInterpolator({GridAxis({1.})}, {GridPointDataSet({1., 1.})}),
                     std::runtime_error);
        , expected_error)
}

TEST(CartesianProduct, cartesian_product)
{
    std::vector<std::vector<short>> result =
        cartesian_product(std::vector<std::vector<short>> {{1, 2, 3}, {4, 5}, {6, 7, 8, 9}});
    EXPECT_EQ(result.size(), 3u * 2u * 4u);
    EXPECT_THAT(result[0], testing::ElementsAre(1, 4, 6));
    EXPECT_THAT(result[1], testing::ElementsAre(1, 4, 7));
    EXPECT_THAT(result[10], testing::ElementsAre(2, 4, 8));
    EXPECT_THAT(result[3 * 2 * 4 - 1], testing::ElementsAre(3, 5, 9));
}

} // namespace Btwxt