/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
 * See the LICENSE file for additional terms and conditions. */

// Vendor
#include <gtest/gtest.h>
#include <btwxt/btwxt.h>
#include <fmt/format.h>
#include <gmock/gmock.h>
#include "fixtures/public-fixtures.h"

using namespace Btwxt;

double fCubic(double x)
{
	return 1.+x*x*x;
}

void Phil_test()
{
		std::vector<double> x_values={0, 2, 5, 10};
		std::vector<double> y_values={6, 5, 4, 3};
		
		/*for(auto &x_value:x_values)
		{
			y_values.push_back(fCubic(x_value));
		}
		*/
	  Method extrapolation_method = Method::linear;
		Method interpolation_method = Method::cubic;

		std::pair<double, double> extrapolation_limits{-DBL_MAX, DBL_MAX};

		std::vector<std::vector<double>> values;
		values.push_back(y_values);

		GridAxis axis(x_values, "x", interpolation_method, extrapolation_method, extrapolation_limits);
		RegularGridInterpolator my_interpolator({axis}, values);

		double x(2.5);
		std::vector<double> target{x};
		my_interpolator.set_target(target);
		double estimate = my_interpolator.get_values_at_target()[0];
		std::cout<<x<< std::fixed << std::setw(11)<<std::setprecision(6)<<"\t"<<"\t"<<estimate<<"\n";

}


int main(int argc, char** argv)
{
 		Phil_test();
 	  ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
