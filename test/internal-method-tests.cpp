/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <chrono>

// vendor
#include <fmt/format.h>

// btwxt
#include "private-fixtures.h"

namespace Btwxt {

TEST_F(CubicFixture, spacing_multiplier) {
  static constexpr std::size_t floor = 0;
  static constexpr std::size_t ceiling = 1;
  double result;
  result = interpolator.get_axis_cubic_spacing_ratios(0, floor)[0];
  EXPECT_DOUBLE_EQ(result, 1.0);

  result = interpolator.get_axis_cubic_spacing_ratios(0, ceiling)[0];
  EXPECT_DOUBLE_EQ(result, (10. - 6.) / (15.0 - 6.0));

  result = interpolator.get_axis_cubic_spacing_ratios(0, floor)[1];
  EXPECT_DOUBLE_EQ(result, (15. - 10.) / (15.0 - 6.0));

  result = interpolator.get_axis_cubic_spacing_ratios(0, ceiling)[2];
  EXPECT_DOUBLE_EQ(result, 1.0);

  result = interpolator.get_axis_cubic_spacing_ratios(1, floor)[0];
  EXPECT_DOUBLE_EQ(result, 1.0);
}

TEST_F(CubicFixture, switch_interp_method) {
  for (auto i = 0u; i < interpolator.number_of_axes; i++) {
    interpolator.set_axis_interpolation_method(i, Method::cubic);
  }
  std::vector<double> result1 = interpolator.get_results(target);
  for (auto i = 0u; i < interpolator.number_of_axes; i++) {
    interpolator.set_axis_interpolation_method(i, Method::linear);
  }
  std::vector<double> result2 = interpolator.get_results(target);
  EXPECT_NE(result1, result2);
}

TEST_F(CubicFixture, interpolate) {
  interpolator.set_target(target);

  auto start = std::chrono::high_resolution_clock::now();
  std::vector<double> result = interpolator.get_results();
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
  BtwxtContextCourierr message_display;
  message_display.info(
      fmt::format("Time to do cubic interpolation: {} microseconds", duration.count()));
  EXPECT_THAT(result, testing::ElementsAre(testing::DoubleEq(4.158), testing::DoubleEq(11.836)));
}

TEST_F(CubicFixture, grid_point_interp_coeffs) {
  static constexpr std::size_t floor = 0;
  static constexpr std::size_t ceiling = 1;

  interpolator.set_target(target);

  double mu = interpolator.floor_to_ceiling_fractions[0];
  std::size_t floor_grid_point_index = interpolator.floor_grid_point_coordinates[0];

  EXPECT_EQ(interpolator.interpolation_coefficients[0][0], 2 * mu * mu * mu - 3 * mu * mu + 1);
  EXPECT_EQ(interpolator.interpolation_coefficients[0][1], -2 * mu * mu * mu + 3 * mu * mu);

  EXPECT_EQ(interpolator.cubic_slope_coefficients[0][0],
            (mu * mu * mu - 2 * mu * mu + mu) *
                interpolator.get_axis_cubic_spacing_ratios(0, floor)[floor_grid_point_index]);
  EXPECT_EQ(interpolator.cubic_slope_coefficients[0][1],
            (mu * mu * mu - mu * mu) *
                interpolator.get_axis_cubic_spacing_ratios(0, ceiling)[floor_grid_point_index]);
}

TEST_F(CubicFixture, hypercube_weigh_one_vertex) {
  interpolator.set_axis_interpolation_method(1, Method::cubic);
  interpolator.set_target(target);
  std::vector<Method> methods = interpolator.methods;

  std::vector<double> mus = interpolator.floor_to_ceiling_fractions;
  double mx = mus[0];
  double my = mus[1];
  double c0x = 2 * mx * mx * mx - 3 * mx * mx + 1;
  double c0y = 2 * my * my * my - 3 * my * my + 1;
  // double c1x = -2*mx*mx*mx + 3*mx*mx;
  double c1y = -2 * my * my * my + 3 * my * my;
  double d0x = mx * mx * mx - 2 * mx * mx + mx;
  double d0y = my * my * my - 2 * my * my + my;
  double d1x = mx * mx * mx - mx * mx;
  double d1y = my * my * my - my * my;
  double s1x = 5.0 / 10;
  double s1y = 2.0 / 4;
  double s0x = 5.0 / 9;
  double s0y = 2.0 / 4;

  std::vector<short> this_vertex = {0, 0};
  double weight = interpolator.get_grid_point_weighting_factor(this_vertex);
  double expected_result = c0x * c0y;
  expected_result += -1 * c0x * d1y * s1y;
  expected_result += -1 * d1x * s1x * c0y;
  expected_result += d1x * s1x * d1y * s1y;
  EXPECT_DOUBLE_EQ(weight, expected_result);

  this_vertex = {-1, 1};
  weight = interpolator.get_grid_point_weighting_factor(this_vertex);
  expected_result = -1 * d0x * s0x * c1y;
  expected_result += -1 * d0x * s0x * d0y * s0y;
  EXPECT_DOUBLE_EQ(weight, expected_result);

  this_vertex = {2, 0};
  weight = interpolator.get_grid_point_weighting_factor(this_vertex);
  expected_result = d1x * s1x * c0y;
  expected_result += -1 * d1x * s1x * d1y * s1y;
  EXPECT_DOUBLE_EQ(weight, expected_result);

  this_vertex = {2, 2};
  weight = interpolator.get_grid_point_weighting_factor(this_vertex);
  expected_result = d1x * s1x * d1y * s1y;
  EXPECT_DOUBLE_EQ(weight, expected_result);
}

TEST_F(CubicFixture, hypercube_calculations) {
  interpolator.set_axis_interpolation_method(1, Method::cubic);
  interpolator.set_target(target);

  auto result = interpolator.get_results();
  EXPECT_NEAR(result[0], 4.1953, 0.0001);
  EXPECT_NEAR(result[1], 11.9271, 0.0001);
}

TEST_F(CubicFixture, get_cubic_spacing_ratios) {
  // for cubic dimension 0: {6, 10, 15, 20}, multipliers should be:
  // floor: {4/4, 5/9, 5/10}
  // ceiling: {4/9, 5/10, 5/5}
  std::vector<std::vector<double>> expected_results{{4.0 / 4, 5.0 / 9, 5.0 / 10},
                                                    {4.0 / 9, 5.0 / 10, 5.0 / 5}};
  double result;
  for (std::size_t floor_or_ceiling = 0; floor_or_ceiling <= 1; floor_or_ceiling++) {
    for (std::size_t index = 0; index < 3; index++) {
      result = interpolator.get_axis_cubic_spacing_ratios(0, floor_or_ceiling)[index];
      EXPECT_DOUBLE_EQ(result, expected_results[floor_or_ceiling][index]);
    }
  }
}

TEST_F(EmptyGridFixturePrivate, locate_coordinates) {
  grid = {{1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 6, 7}};
  setup();

  std::vector<std::size_t> coords = {2, 3};
  std::vector<std::size_t> axis_lengths = {5, 7};
  std::size_t index = interpolator.get_grid_point_index(coords);
  EXPECT_EQ(index, 17u);

  coords = {2, 3, 2};
  grid = {{1, 2, 3, 4, 5}, {1, 2, 3, 4, 5, 6, 7}, {1, 2, 3}};
  setup();

  index = interpolator.get_grid_point_index(coords);
  EXPECT_EQ(index, 53u);
}

TEST_F(EmptyGridFixturePrivate, set_axis_floor) {

  grid = {{1, 3, 5, 7, 9}};
  setup();
  interpolator.set_axis_extrapolation_limits(0, {0, 11});

  interpolator.set_target({5.3});

  interpolator.set_floor_grid_point_coordinates();

  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::in_bounds);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 2u);

  interpolator.set_target({0.3});
  interpolator.set_floor_grid_point_coordinates();
  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::out_of_bounds);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 0u);

  interpolator.set_target({10.3});
  interpolator.set_floor_grid_point_coordinates();
  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::out_of_bounds);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 3u);

  interpolator.set_target({-0.3});
  interpolator.set_floor_grid_point_coordinates();
  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::outlaw);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 0u);

  interpolator.set_target({11.3});
  interpolator.set_floor_grid_point_coordinates();
  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::outlaw);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 3u);

  interpolator.set_axis_extrapolation_limits(0, {-DBL_MAX, DBL_MAX});
  interpolator.set_target({-0.3});
  interpolator.set_floor_grid_point_coordinates();
  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::out_of_bounds);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 0u);

  interpolator.set_target({11.3});
  interpolator.set_floor_grid_point_coordinates();
  EXPECT_EQ(interpolator.is_inbounds[0], Bounds::out_of_bounds);
  EXPECT_EQ(interpolator.floor_grid_point_coordinates[0], 3u);
}

TEST_F(GridFixture2DPrivate, grid_point_basics) {
  interpolator.set_target(target);

  std::vector<std::size_t> expected_floor{1, 0};
  EXPECT_EQ(interpolator.floor_grid_point_coordinates, expected_floor);

  std::vector<double> expected_floor_to_ceiling_fractions{0.4, 0.5};
  EXPECT_EQ(interpolator.floor_to_ceiling_fractions, expected_floor_to_ceiling_fractions);
}

TEST_F(GridFixture2DPrivate, grid_point_out_of_bounds) {
  std::vector<double> out_of_bounds_vector = {16, 3};
  interpolator.set_target(out_of_bounds_vector);

  std::vector<std::size_t> expected_floor{1, 0};
  EXPECT_EQ(interpolator.floor_grid_point_coordinates, expected_floor);

  std::vector<double> expected_floor_to_ceiling_fractions{1.2, -0.5};
  EXPECT_EQ(interpolator.floor_to_ceiling_fractions, expected_floor_to_ceiling_fractions);
}

TEST_F(GridFixture2DPrivate, consolidate_methods) {
  interpolator.set_target(target);

  std::vector<Method> expected_methods{Method::linear, Method::linear};
  EXPECT_EQ(interpolator.methods, expected_methods);

  std::vector<double> out_of_bounds_vector = {12, 3};
  interpolator.set_target(out_of_bounds_vector);
  interpolator.methods = interpolator.methods;
  expected_methods = {Method::linear, Method::constant};
  EXPECT_EQ(interpolator.methods, expected_methods);
}

TEST_F(GridFixture2DPrivate, interpolation_coefficients) {
  interpolator.set_target(target);

  std::vector<double> mu = interpolator.floor_to_ceiling_fractions;

  EXPECT_EQ(interpolator.interpolation_coefficients[0][1], mu[0]);
  EXPECT_EQ(interpolator.interpolation_coefficients[1][0], 1 - mu[1]);

  EXPECT_EQ(interpolator.cubic_slope_coefficients[0][0], 0);
  EXPECT_EQ(interpolator.cubic_slope_coefficients[1][1], 0);
}

TEST_F(GridFixture2DPrivate, construct_from_axes) {
  GridAxis ax0 = GridAxis(std::vector<double>({0, 10, 15}));
  auto logger = ax0.get_logger();
  GridAxis ax1 = GridAxis(std::vector<double>({4, 6}), logger);
  std::vector<GridAxis> test_axes = {ax0, ax1};
  interpolator = RegularGridInterpolatorPrivate(test_axes, logger);
  EXPECT_EQ(interpolator.number_of_axes, 2u);
  EXPECT_EQ(interpolator.number_of_tables, 0u);
  EXPECT_THAT(interpolator.axis_lengths, testing::ElementsAre(3, 2));

  interpolator.add_value_table(values[0]);
  EXPECT_EQ(interpolator.number_of_tables, 1u);
  std::vector<std::size_t> coords{1, 1};
  EXPECT_THAT(interpolator.get_grid_point_values(coords), testing::ElementsAre(8));

  interpolator = RegularGridInterpolatorPrivate(test_axes, values, logger);
  EXPECT_EQ(interpolator.number_of_axes, 2u);
  EXPECT_EQ(interpolator.number_of_tables, 2u);
  EXPECT_THAT(interpolator.get_grid_point_values(coords), testing::ElementsAre(8, 16));
}

TEST_F(GridFixture2DPrivate, get_axis_values) {
  std::vector<double> returned_vec = interpolator.get_axis_values(1);
  EXPECT_THAT(returned_vec, testing::ElementsAre(4, 6));
}

TEST_F(GridFixture2DPrivate, get_grid_point_values) {
  std::vector<std::size_t> coords = {0, 1};
  std::vector<double> returned_vec = interpolator.get_grid_point_values(coords);
  EXPECT_THAT(returned_vec, testing::ElementsAre(3, 6));

  coords = {1, 0};
  returned_vec = interpolator.get_grid_point_values(coords);
  EXPECT_THAT(returned_vec, testing::ElementsAre(2, 4));
}

TEST_F(GridFixture2DPrivate, get_grid_point_values_relative) {
  std::vector<std::size_t> coords{0, 1};
  std::vector<short> translation{1, 0}; // {1, 1} stays as is
  std::vector<double> expected_vec = interpolator.get_grid_point_values({1, 1});
  EXPECT_EQ(interpolator.get_grid_point_values_relative(coords, translation), expected_vec);

  translation = {1, 1}; // {1, 2} -> {1, 1}
  EXPECT_EQ(interpolator.get_grid_point_values_relative(coords, translation), expected_vec);

  translation = {-1, 0}; // {-1, 1} -> {0, 1}
  expected_vec = interpolator.get_grid_point_values({0, 1});
  EXPECT_EQ(interpolator.get_grid_point_values_relative(coords, translation), expected_vec);

  translation = {3, -2}; // {3, -1} -> {2, 0}
  expected_vec = interpolator.get_grid_point_values({2, 0});
  EXPECT_EQ(interpolator.get_grid_point_values_relative(coords, translation), expected_vec);
}

TEST(Fractions, compute_fraction) {
  double floor_to_ceiling_fraction = compute_fraction(4.3, 4, 6);
  EXPECT_DOUBLE_EQ(floor_to_ceiling_fraction, 0.15);
}

TEST_F(GridFixture3DPrivate, hypercube) {
  auto hypercube = interpolator.get_hypercube();
  EXPECT_EQ(hypercube.size(), 16u);
}

TEST_F(GridFixture3DPrivate, test_hypercube) {
  auto hypercube = interpolator.get_hypercube();
  EXPECT_EQ(hypercube.size(), 2u * 4u * 2u);
  EXPECT_THAT(hypercube[0], testing::ElementsAre(0, -1, 0));
  EXPECT_THAT(hypercube[2], testing::ElementsAre(0, 0, 0));
  EXPECT_THAT(hypercube[12], testing::ElementsAre(1, 1, 0));
  EXPECT_THAT(hypercube[15], testing::ElementsAre(1, 2, 1));
}

TEST_F(GridFixture3DPrivate, make_linear_hypercube) {
  interpolator.set_axis_interpolation_method(1, Method::linear);
  auto hypercube = interpolator.get_hypercube();
  EXPECT_EQ(hypercube.size(), 8u);
  EXPECT_THAT(hypercube[0], testing::ElementsAre(0, 0, 0));
  EXPECT_THAT(hypercube[2], testing::ElementsAre(0, 1, 0));
  EXPECT_THAT(hypercube[5], testing::ElementsAre(1, 0, 1));
}

} // namespace Btwxt