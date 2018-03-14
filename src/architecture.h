// header file-ish to try out architecture

#include<vector>



class GridAxis{
  GridAxis(double *grid, std::size_t size);
  GridAxis(std::vector<double> grid_vector);

  double *grid;
  std::size_t size;

  std::pair<std::size_t, double> find_floor_and_weight(double target);
};

class GridAxes{
  GridAxes(std::size_t ndim, std::vector<GridAxis>);

  std::size_t ndims;
  std::vector<std::size_t> dim_lengths;

  std::vector< std::pair<std::size_t, double> > find_floors_and_weights(double target[]);
};

class PerformancePoint{
  PerformancePoint(double target[]);
  double target;
  std::vector<std::size_t> floor;
  std::vector<double> weight;
};

class ValueTable{
  ValueTable(double *grid);
  ValueTable(std::vector<double> grid);

  double values[];
  double get_value(std::vector<std::size_t> coords);
  double evaluate_linear(PerformancePoint target);
  // collect_hypercube
};


class RegularGridInterpolator{
  RegularGridInterpolator(
    std::vector< std::vector<double> > grid,
    std::vector<double> values);
  RegularGridInterpolator(
    std::vector< std::vector<double> > grid,
    double values[], std::size_t size);
  RegularGridInterpolator(
    std::vector< double[] > grid,
    double values[], std::size_t size);

  GridAxes grid_axes;
  ValueTable values;

  double get_value(std::vector<std::size_t> coords);  // wraps ValueTable.get_value()

  double calculate_value_at_target(std::vector<double> target);
  double operator()(std::vector<double> target)
  {
    return calculate_value_at_target(target);
  }

  double calculate_value_at_target(PerformancePoint tp);
  double operator()(PerformancePoint tp)
  {
    return calculate_value_at_target(tp);
  }
};
