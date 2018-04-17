/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDDEDDATA_H_
#define GRIDDEDDATA_H_

#include<vector>
#include "Eigen/Dense"


namespace Btwxt {

    const int CONSTANT = 0;  // constant extrapolation
    const int LINEAR = 1;    // linear interpolation/extrapolation
    const int CUBIC = 2;     // cubic spline interpolation (Catmull-Rom)

    class GridAxis {
        // A single input dimension of the performance space
    public:
        GridAxis();

        GridAxis(double *grid, std::size_t size,
                 int extrapolation_method = CONSTANT,
                 int interpolation_method = LINEAR);

        GridAxis(std::vector<double> grid_vector,
                 int extrapolation_method = CONSTANT,
                 int interpolation_method = LINEAR);

        const std::vector<double> grid;
        std::vector<std::vector<double> > spacing_multiplier;
        int extrapolation_method;
        int interpolation_method;
        // std::pair<double> extrapolation_bounds;  <-- to add later
        // bool is_regular;  <-- to add later

        std::size_t get_length();

        void set_interp_method(const int interpolation_method);

        double get_spacing_multiplier(const std::size_t &flavor, const std::size_t &index);

    private:
        std::vector<std::vector<double> > calc_spacing_multipliers();
    };


    class GriddedData {
    public:
        GriddedData();

        GriddedData(
                std::vector<std::vector<double> > grid,
                std::vector<std::vector<double> > values
        );

        std::size_t get_ndims();

        std::size_t get_num_tables();

        std::vector<std::size_t> get_dim_lengths();

        std::vector<double> get_grid_vector(const std::size_t &dim);

        std::vector<double> get_values(const std::vector<std::size_t> &coords);

        template <typename T>
        Eigen::ArrayXd get_column(const std::vector<T> &coords);

        Eigen::ArrayXd get_column_near(
                std::vector<std::size_t> coords, const std::size_t &dim, const int &i);

        Eigen::ArrayXd get_column_near(
                std::vector<std::size_t> coords, const std::vector<int> &translation);

        Eigen::ArrayXd get_column_near_safe(
                const std::vector<std::size_t>& coords, std::vector<int> translation);

        Eigen::ArrayXd get_column_up(
                const std::vector<std::size_t> &coords, const std::size_t &dim);

        Eigen::ArrayXd get_column_down(
                const std::vector<std::size_t> &coords, const std::size_t &dim);

        double get_axis_spacing_mult(const std::size_t &dim,
                                     const std::size_t &flavor, const std::size_t &index);

        std::vector<int> get_interp_methods();

        std::vector<int> get_extrap_methods();
        // double get_value(std::size_t table_index, std::vector<std::size_t> coords);

        void set_axis_extrap_method(const std::size_t &dim, const int);

        void set_axis_interp_method(const std::size_t &dim, const int);

        Eigen::ArrayXXd value_tables;
        std::size_t num_values;
        std::vector<std::size_t> dimension_lengths;

    private:
        void construct_axes(const std::vector<std::vector<double> > &grid);

        Eigen::ArrayXXd construct_values(
                const std::vector<std::vector<double> > &values
        );

        std::vector<GridAxis> grid_axes;
        std::size_t ndims;
        std::size_t num_tables;
        // TODO move interp_method down to GridAxis
        std::vector<int> interp_methods;
    };


// free functions
    bool free_check_sorted(std::vector<double>);

    template <typename T>
    std::size_t locate_coords(
            const std::vector<T> &coords,
            const std::vector<std::size_t> &dimension_lengths
    );

    std::vector<double> eigen_to_vector(Eigen::ArrayXd &);


}
#endif // GRIDDEDDATA_H_
