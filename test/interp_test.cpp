

// Standard
#include<iostream>
#include "gtest/gtest.h"

// example: https://github.com/Crascit/DownloadProject/blob/master/example.cpp
// example2: https://github.com/bigladder/kiva/blob/develop/test/unit/foundation.unit.cpp

// btwxt
#include <longtable.h>



class BaseFixture : public testing::Test {
protected:
  BaseFixture(){
    std::vector<double> first = {12.77778, 15, 18, 19.44448943, 21, 23.88889};
    std::vector<double> second = {18, 24, 30, 35, 36, 41, 46.11111};
    std::vector< std::vector<double> > grid = {first, second};
    double values[] = {
      24421.69383, 25997.3589, 28392.31868, 29655.22876, 31094.97495, 33988.3473,
      22779.73113, 24352.1562, 26742.74198, 28003.546, 29441.02425, 32330.1846,
      21147.21662, 22716.4017, 25102.61348, 26361.31143, 27796.52175, 30681.4701,
      19794.00525, 21360.49033, 23743.0571, 25000, 26433.32038, 29314.75872,
      19524.15032, 21090.0954, 23471.93318, 24728.52506, 26161.46745, 29042.2038,
      18178.81244, 19742.05753, 22120.2503, 23375.08713, 24806.13958, 27683.36592,
      16810.36004, 18370.84513, 20745.3119, 21998.35468, 23427.47518, 26301.11353
      };

    my_longtable = Interp::LongTable(grid, values);
  }

  Interp::LongTable my_longtable;
};



double interpolate(double t, double a0, double a1) {
  return t*a1 + (1-t)*a0;
}

TEST(interpolate, two_numbers) {
  double x = interpolate(0.2, 5, 10);
  EXPECT_EQ(x, 6);
}

TEST_F(BaseFixture, ndims) {

  std::size_t ndims = my_longtable.get_ndims();
  EXPECT_EQ(ndims, 2);
}

TEST_F(BaseFixture, nvalues) {

  std::size_t nvalues = my_longtable.get_nvalues();
  EXPECT_EQ(nvalues, 42);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  std::cout << "Hello World" << std::endl;

  std::vector<double> first = {12.77778, 15, 18, 19.44448943, 21, 23.88889};
  std::vector<double> second = {18, 24, 30, 35, 36, 41, 46.11111};
  std::vector< std::vector<double> > grid = {first, second};
  double values[] = {
    24421.69383, 25997.3589, 28392.31868, 29655.22876, 31094.97495, 33988.3473,
    22779.73113, 24352.1562, 26742.74198, 28003.546, 29441.02425, 32330.1846,
    21147.21662, 22716.4017, 25102.61348, 26361.31143, 27796.52175, 30681.4701,
    19794.00525, 21360.49033, 23743.0571, 25000, 26433.32038, 29314.75872,
    19524.15032, 21090.0954, 23471.93318, 24728.52506, 26161.46745, 29042.2038,
    18178.81244, 19742.05753, 22120.2503, 23375.08713, 24806.13958, 27683.36592,
    16810.36004, 18370.84513, 20745.3119, 21998.35468, 23427.47518, 26301.11353
    };

  Interp::LongTable my_longtable(grid, values);
  std::size_t ndims = my_longtable.get_ndims();
  std::cout << "dimensions = " << ndims << std::endl;
  std::cout << "values in table = " << my_longtable.get_nvalues() << std::endl;
  std::vector<size_t> dim_lengths = my_longtable.get_dim_lengths();
  std::cout << "dimension lengths: " << dim_lengths[0] << ", " << dim_lengths[1] << std::endl;

  // expecting 23743.0571
  std::cout << "values(2,3) = " << my_longtable.get_value(2, 3) << std::endl;
  std::vector<size_t> x = {2, 3};
  std::cout << "values(2,3) = " << my_longtable.get_value_gen(x) << std::endl;

  // expecting 16810.36004
  std::cout << "values(0,6) = " << my_longtable.get_value(0, 6) << std::endl;
  x = {0, 6};
  std::cout << "values(0,6) = " << my_longtable.get_value_gen(x) << std::endl;

  // prototype here before moving to longtable.cpp
  // coords is the point we are interpolating to
  double coords[] = {20, 31};
  // TODO: dynamic array sizing to ndim
  std::vector<size_t> floors;
  for (std::size_t d=0; d<ndims; d+=1) {
    floors.push_back(my_longtable.grid_floor(coords[d], d));
    std::cout << coords[d] << " is greater than item " << floors[d]
              << " in dim "<< d << ": " << grid[d][floors[d]]<< std::endl;
  }

  // get fractions of the grid-space crossing on each axis
  std::vector<double> fracs;
  // double fracs[2];
  for (std::size_t d=0; d<ndims; d+=1) {
    fracs.push_back(my_longtable.get_fraction(coords[d], d));
    // fracs[d] = (coords[d] - grid[d][floors[d]]) / (grid[d][floors[d]+1] - grid[d][floors[d]]);
  }
  std::cout << fracs[0] << ", " << fracs[1] << std::endl;

  // interpolate first dimension
  double intermediates[2];
  for (std::size_t d=0; d<ndims-1; d+=1) {
    intermediates[0] = interpolate(fracs[d], my_longtable.get_value(floors[d], floors[d+1]),
                        my_longtable.get_value(floors[d]+1, floors[d+1]));
    intermediates[1] = interpolate(fracs[d], my_longtable.get_value(floors[d], floors[d+1]+1),
                        my_longtable.get_value(floors[d]+1, floors[d+1]+1));
  }

  // interpolate final dimension
  double result = interpolate(fracs[1], intermediates[0], intermediates[1]);
  std::cout << intermediates[0] << ", " << intermediates[1] << ", " << result << std::endl;


  // std::cout << interpolate(fracs[0], my_longtable.get_value(floors[0], floors[1]),
  //              my_longtable.get_value(floors[0]+1, floors[1])) << std::endl;

  //
  // intermediate0 = my_longtable.get_value(floor0, floor1) +
  // frac1*(my_longtable.get_value(floor0, floor1+1) - my_longtable.get_value(floor0, floor1));
  // intermediate1 = my_longtable.get_value(floor0+1, floor1) +
  // frac1*(my_longtable.get_value(floor0+1, floor1+1) - my_longtable.get_value(floor0+1, floor1));
  //
  // result = intermediate0 + frac0*(intermediate1 - intermediate0);
  // std::cout << intermediate0 << ", " << intermediate1 << ", " << result << std::endl;

  return RUN_ALL_TESTS();
}
