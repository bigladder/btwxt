/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Standard
#include <iostream>
#include <memory>

// Vendor
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// btwxt
#include <btwxt/btwxt.h>

// testing
#include "fixtures/public-fixtures.h" // EXPECT_STDOUT

namespace Btwxt {

TEST(GridAxis, vector_is_sorted)
{
    std::vector<std::pair<std::vector<double>, bool>> axes = {{{1, 3, 5, 7, 9}, true},
                                                              {{1, 3, 5, 17, 9}, false},
                                                              {{9, 7, 5, 3, 1}, false},
                                                              {{1, 3, 3, 7, 9}, false},
                                                              {{9}, true}};
    bool is_sorted;
    for (const auto& pair : axes) {
        is_sorted = Btwxt::vector_is_sorted(pair.first);
        EXPECT_EQ(is_sorted, pair.second);
    }
}

TEST(GridAxis, sorting)
{
    std::vector<double> grid_vector = {0, 5, 7, 17, 15};
    EXPECT_THROW(GridAxis my_grid_axis = GridAxis(grid_vector), BtwxtException);
    grid_vector = {0, 5, 7, 10, 15};
    EXPECT_NO_THROW(GridAxis my_grid_axis = GridAxis(grid_vector););
}

TEST(GridAxis, calculate_cubic_spacing_ratios)
{
    static constexpr std::size_t i_interval = 2;
  
    std::vector<double> grid_values={6., 10., 15., 20., 22.};
    std::vector<SlopeMethod> slope_method_vector
    ({  SlopeMethod::quadratic,
        SlopeMethod::finite_diff,
        SlopeMethod::cardinal,
    });
    double slope_reduction = 0.0;
    for (auto slope_method: slope_method_vector)
    {
        GridAxis grid_axis(grid_values,
                           "",
                           Method::cubic,
                           Method::constant,
                           {-DBL_MAX, DBL_MAX},
                           slope_method,
                           slope_reduction,
                           std::make_shared<BtwxtLogger>());
        
        std::vector<std::pair<double,double>> result(4, {0., 0.});
        
        double w_0 = grid_values[i_interval + 1] - grid_values[i_interval];
        if ((i_interval > 0) && (i_interval + 1 < grid_values.size())){
            double w_m1 = grid_values[i_interval] - grid_values[i_interval - 1];
            double t_0 = w_0 / w_m1;
            
            double beta_0(0.0);
            switch (slope_method){
                case SlopeMethod::finite_diff:
                {
                    beta_0 = 0.5;
                    break;
                }
                case SlopeMethod::cardinal:{
                    beta_0 = 1 / (1 + t_0);
                    break;
                }
                case SlopeMethod::quadratic:
                default:{
                    beta_0 = t_0 / (1 + t_0);
                    break;
                }
            }

            //general
            double s_m1_m = (1 - slope_reduction) * (-t_0 * beta_0);
            double s_1_m = (1 - slope_reduction) * (1 - beta_0);
            result[0].first =  s_m1_m;
            result[1].first = -(s_m1_m + s_1_m);
            result[2].first = s_1_m;
        }
        
        if (i_interval + 2 < grid_values.size()){
            double w_1 = grid_values[i_interval + 2] - grid_values[i_interval + 1];
            double t_1 = w_1 / w_0;
            
            double beta_1(0.0);
            switch (slope_method){
                case SlopeMethod::finite_diff:{
                    beta_1 = 0.5;
                    break;
                }
                case SlopeMethod::cardinal:{
                    beta_1 = 1 / (1 + t_1);
                    break;
                }
                case SlopeMethod::quadratic:
                default:{
                    beta_1 = t_1 / (1 + t_1);
                    break;
                }
            }

            double s_0_p = (1 - slope_reduction) * (-beta_1);
            double s_2_p = (1 - slope_reduction) * (1 - beta_1) / t_1;
            
            result[1].second = s_0_p;
            result[2].second = -(s_0_p + s_2_p);
            result[3].second = s_2_p;
        }
        
        auto& expected=grid_axis.get_cubic_spacing_ratios(i_interval);
        
        for (std::size_t i = 0; i < 4; ++i){
            EXPECT_NEAR(expected[i].first, result[i].first, 0.0001);
            EXPECT_NEAR(expected[i].second, result[i].second, 0.0001);
        }
    }
}

TEST(GridAxis, bad_limits)
{
    GridAxis my_grid_axis({0, 5, 7, 11, 12, 15});
    my_grid_axis.name = "my grid axis";
    std::pair<double, double> extrapolation_limits {4, 17};
    std::string expected_out =
        "  [NOTE] Grid axis (name=\"my grid axis\") lower extrapolation limit (4) is within the "
        "set of grid axis values. Setting to smallest axis value (0).\n";
    EXPECT_STDOUT(my_grid_axis.set_extrapolation_limits(extrapolation_limits);, expected_out)
    EXPECT_EQ(my_grid_axis.get_extrapolation_limits().first, 0);

    extrapolation_limits = {-2, 12};
    expected_out =
        "  [NOTE] Grid axis (name=\"my grid axis\") upper extrapolation limit (12) is within the "
        "set of grid axis values. Setting to largest axis value (15).\n";
    EXPECT_STDOUT(my_grid_axis.set_extrapolation_limits(extrapolation_limits);, expected_out)
    EXPECT_EQ(my_grid_axis.get_extrapolation_limits().second, 15);
}
} // namespace Btwxt
