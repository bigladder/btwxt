

// Standard
#include<iostream>
#include "gtest/gtest.h"

// example: https://github.com/Crascit/DownloadProject/blob/master/example.cpp
// example2: https://github.com/bigladder/kiva/blob/develop/test/unit/foundation.unit.cpp

// btwxt
#include <gridinterp.h>



class BaseFixture : public testing::Test {
protected:
  Btwxt::RegularGridInterpolator test_rgi;
  std::vector<double> target;

  BaseFixture(){
    std::vector<double> first = {12.77778, 15, 18, 19.44448943, 21, 23.88889};
    std::vector<double> second = {18, 24, 30, 35, 36, 41, 46.11111};
    std::vector< std::vector<double> > grid = {first, second};
    std::vector<double> values = {
      24421.69383, 25997.3589, 28392.31868, 29655.22876, 31094.97495, 33988.3473,
      22779.73113, 24352.1562, 26742.74198, 28003.546, 29441.02425, 32330.1846,
      21147.21662, 22716.4017, 25102.61348, 26361.31143, 27796.52175, 30681.4701,
      19794.00525, 21360.49033, 23743.0571, 25000, 26433.32038, 29314.75872,
      19524.15032, 21090.0954, 23471.93318, 24728.52506, 26161.46745, 29042.2038,
      18178.81244, 19742.05753, 22120.2503, 23375.08713, 24806.13958, 27683.36592,
      16810.36004, 18370.84513, 20745.3119, 21998.35468, 23427.47518, 26301.11353
      };

    target = {20, 31};
    test_rgi = Btwxt::RegularGridInterpolator(grid, values);
  }
};


TEST_F(BaseFixture, interpolate) {
  double x = Btwxt::interpolate(0.2, 5, 10);
  EXPECT_EQ(x, 6);
}

TEST_F(BaseFixture, raise_to_power) {
  EXPECT_EQ(Btwxt::pow(2, 5), 32);
  EXPECT_EQ(Btwxt::pow(3, 4), 81);
  EXPECT_EQ(Btwxt::pow(7, 1), 7);
  EXPECT_EQ(Btwxt::pow(4, 0), 1);
}

TEST_F(BaseFixture, ndims) {

  std::size_t ndims = test_rgi.get_ndims();
  EXPECT_EQ(ndims, 2);
}

TEST_F(BaseFixture, nvalues) {

  std::size_t nvalues = test_rgi.get_nvalues();
  EXPECT_EQ(nvalues, 42);
}

TEST_F(BaseFixture, find_floor) {
  std::size_t floor = test_rgi.grid_floor(20, 0);
  EXPECT_EQ(floor, 3);
}

TEST_F(BaseFixture, can_interpolate) {
  double result = test_rgi.calculate_value_at_target(target);
  EXPECT_NEAR(result, 26601.5, 0.1);
}

TEST_F(BaseFixture, function_interpolate) {
  Btwxt::RegularGridInterpolator my_interpolating_function = test_rgi;
  double result = my_interpolating_function(target);
  EXPECT_NEAR(result, 26601.5, 0.1);
}



int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  std::cout << "Hello World" << std::endl;

  return RUN_ALL_TESTS();
}
