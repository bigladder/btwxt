/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

#ifndef HYPER_H_
#define HYPER_H_

#include<vector>
#include "Eigen/Dense"

// btwxt
#include "griddeddata.h"
#include "gridpoint.h"

namespace Btwxt {


    // TODO: delete CoreHypercube. I don't think we need it anymore.
    class CoreHypercube {
    public:
        CoreHypercube();

        CoreHypercube(const std::size_t &ndims);

        std::size_t ndims;
        std::vector<std::vector<int> > vertices;

        void collect_things(WhereInTheGridIsThisPoint &);

        Eigen::ArrayXd compute_core(GriddedData &);

        double weigh_one_vertex(const std::vector<int> &v);

        std::vector<std::size_t> point_floor;
        std::vector<int> methods;
        std::vector<std::vector<double> > interp_coeffs;
        std::vector<std::vector<double> > cubic_slope_coeffs;
    };

    class FullHypercube {
    public:
        FullHypercube();

        FullHypercube(const std::size_t &ndims, const std::vector<int> &methods);

        void collect_things(WhereInTheGridIsThisPoint &);

        std::size_t ndims;
        std::vector<std::vector<int> > vertices;

        Eigen::ArrayXd all_the_calculations(GriddedData &the_blob);

        std::vector< std::vector<double> > get_spacing_mults(GriddedData &the_blob);

        double weigh_one_vertex(const std::vector<int> &v,
                const std::vector< std::vector<double> >& spacing_mults);

        std::vector<std::size_t> point_floor;
        std::vector<int> methods;
        std::vector<std::vector<double> > cubic_slope_coeffs;
        std::vector<std::vector<double> > interp_coeffs;
        std::vector< std::pair<int, int> > sivor;  // {sign, flavor}
    };


// free functions
    std::vector<std::vector<int> > make_core_hypercube(const std::size_t &ndims);

    std::vector<std::vector<int> > make_full_hypercube(
            const std::size_t &ndims, const std::vector<int> &fit_degrees);

    template <typename T>
    std::vector<std::vector<T> > cart_product(const std::vector<std::vector<T> > &v);

    std::vector<double> cart_product_m(const std::vector<std::vector<double> > &v);

}
#endif // HYPER_H_
