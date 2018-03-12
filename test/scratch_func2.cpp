#include<iostream>
#include<vector>

class RegularGridInterpolator{
public:
  RegularGridInterpolator(
    std::vector<double> grid,
    std::vector<double> values);

  std::vector<double> grid;
  std::vector<double> values;

  double interpolate(double target);
  double operator()(double target)
  {
    return interpolate(target);
  }
};


RegularGridInterpolator::RegularGridInterpolator(
  std::vector<double> grid,
  std::vector<double> values) :
grid(grid),
values(values)
{

};

double RegularGridInterpolator::interpolate(double target) {
  double t = (target-grid[0]) / (grid[1] - grid[0]);
  return t*values[1] + (1-t)*values[0];
}

int main()
{
  std::vector<double> grid = {0, 10};
  std::vector<double> values = {12, 24};
  RegularGridInterpolator my_interpolating_function(grid, values);

  double target = 5;
  double result = my_interpolating_function(target);
  std::cout << "the interpolated value at " << target << " is " << result << std::endl;

  target = 9;
  result = my_interpolating_function(target);
  std::cout << "the interpolated value at " << target << " is " << result << std::endl;


  std::cout << "second interpolation field" << std::endl;
  grid = {5, 10};
  values = {1, 7};
  RegularGridInterpolator second_interpolating_function(grid, values);

  target = 6;
  result = second_interpolating_function(target);
  std::cout << "the interpolated value at " << target << " is " << result << std::endl;

  return 0;
};
