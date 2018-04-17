/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef GRIDPOINT_H_
#define GRIDPOINT_H_

// Standard
#include<vector>

// btwxt
#include "griddeddata.h"


namespace Btwxt {

    class GridPoint {
    public:
        // target is an array of doubles specifying the point we are interpolating to.
        GridPoint();

        GridPoint(double *target);

        GridPoint(const std::vector<double> &target_vector);

        std::vector<double> target;
    };


    class WhereInTheGridIsThisPoint {
    public:
        WhereInTheGridIsThisPoint();

        WhereInTheGridIsThisPoint(GridPoint &, GriddedData &);

        std::vector<std::size_t> get_floor();

        std::vector<double> get_weights();

        std::vector<bool> get_is_inbounds();

        std::vector<int> get_methods();

        std::vector<std::vector<double> > get_interp_coeffs();

        std::vector<std::vector<double> > get_cubic_slope_coeffs();

    private:
        std::size_t ndims;
        std::vector<std::size_t> point_floor;  // index of grid point <= target
        std::vector<double> weights;
        // TODO upgrade is_inbounds to a family of const ints to allow both
        //     1. outside grid but can extrapolate to, and
        //     2. outside allowed extrapolation zone.
        std::vector<bool> is_inbounds;  // for deciding interpolation vs. extrapolation;
        std::vector<int> methods;
        std::vector<std::vector<double> > interp_coeffs;
        std::vector<std::vector<double> > cubic_slope_coeffs;

        void find_floor(
                std::vector<std::size_t> &point_floor, std::vector<bool> &is_inbounds,
                GridPoint &, GriddedData &);

        void calculate_weights(
                const std::vector<std::size_t> &point_floor, std::vector<double> &weights,
                GridPoint &, GriddedData &);

        void consolidate_methods(const std::vector<int> &interp_methods,
                                 const std::vector<int> &extrap_methods);

        void calculate_interp_coeffs();
    };

}
#endif // GRIDPOINT_H_
