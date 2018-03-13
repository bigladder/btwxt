/* Copyright (c) 2018 Big Ladder Software LLC. All rights reserved.
* See the LICENSE file for additional terms and conditions. */

// Standard
#include<iostream>

//btwxt
#include "gridinterp.h"

namespace Btwxt {


/*  ##  assumptions we are making  ##
1. grid vectors line up with nvalues
  a. number of dimensions = number of grid vectors
  b. nvalues = product of dimension lengths
  c. grid arrives in the appropriate order <- have to assume this, i think.
2. grid vectors arrive sorted
3. target length = ndims
4. target falls within grid.  TODO extrapolation
5. target is in the appropriate order <- have to assume this, too
*/


// some free functions
double interpolate(double t, double a0, double a1) {
  // general linear interpolation in one dimension
  return t*a1 + (1-t)*a0;
}

std::size_t pow(std::size_t base, std::size_t power) {
  // raise base to a power (both must be size_t)
  if (power == 0) {return 1;}
  else {
    std::size_t result = base;
    for (std::size_t i=1; i<power; i++)
    {
        result = result*base;
    }
    return result;
  }
}

std::vector< std::vector<std::size_t> > make_binary_list(std::size_t ndims) {
  // produces a list of binary representations of numbers up to 2^ndims.
  // e.g., if ndims=2, this function returns {{0,0}, {0,1}, {1,0}, {1,1}}
  // these binary representations are used to collect all of the points in the interpolation hypercube
  std::vector< std::vector<std::size_t> > binaries;
  for (std::size_t n=0; n<pow(2, ndims); n++) {
    std::size_t i;
    std::size_t b;
    std::vector<std::size_t> single;
    for (i = 1 << (ndims-1); i > 0; i = i / 2) {
      (n & i)? b=1: b=0;
      single.push_back(b);
    }
    binaries.push_back(single);
    if (ndims==2) {
      std::cout << n << " = "<< binaries[n][0] << ", " << binaries[n][1] << std::endl;
    }
    else if (ndims==3) {
      std::cout << n << " = "<< binaries[n][0] << ", " << binaries[n][1] << ", " << binaries[n][2] << std::endl;
    }
  }
  return binaries;
}

std::vector<double> collapse_dimension(std::vector<double> input, double frac) {
  // interpolate along one axis of an n-dimensional hypercube.
  // this flattens a square to a line, or a cube to a square, etc.
  std::vector<double> output;
  for (std::size_t i=0; i<input.size(); i += 2) {
    output.push_back(interpolate(frac, input[i], input[i+1]));
    std::cout << input[i] << " & " << input[i+1] << " => " << interpolate(frac, input[i], input[i+1]) << std::endl;
  }
  return output;
}



RegularGridInterpolator::RegularGridInterpolator()
{

};

// RegularGridInterpolator::RegularGridInterpolator(
//   std::vector< std::vector<double> > grid,
//   const double* values) :
// grid(grid),
// values = vec(values)  // this is an initializer list, apparently
// {
//     ndims = grid.size();
//     nvalues = 1;
//     for(auto dim : grid){
//       dim_lengths.push_back(dim.size());
//       nvalues *= dim.size();
//     }
// };

RegularGridInterpolator::RegularGridInterpolator(
    std::vector< std::vector<double> > grid,
    std::vector<double> values) :
  grid(grid),
  values(values)
  {
    ndims = grid.size();
    nvalues = 1;
    for(auto dim : grid){
      dim_lengths.push_back(dim.size());
      nvalues *= dim.size();
    }
    std::cout << "we have a " << ndims << "D table with " << nvalues << " values." << std::endl;
    std::cout << "the table dimensions are:" << std::endl;
    for (auto dim : dim_lengths) {
      std::cout << dim << std::endl;
    }
  };


  std::size_t RegularGridInterpolator::get_ndims()
  {
    return ndims;
  }

  std::size_t RegularGridInterpolator::get_nvalues()
  {
    return nvalues;
  }

  std::vector<size_t> RegularGridInterpolator::get_dim_lengths()
  {
    return dim_lengths;
  }


  double RegularGridInterpolator::calculate_value_at_target(std::vector<double> target)
  {
    std::cout << "\nthe interpolation target, and how it fits on the grid: " << std::endl;
    std::vector<size_t> floor = find_floor(target);

    // get fractions of the grid-space crossing on each axis
    std::cout << "\nhow far in the hypercube in each dimension: " << std::endl;
    std::vector<double> weights = set_weights(target);

    // collect all of the points in the interpolation hypercube
    std::vector<double> hypercube;

    std::cout << "\nwe use binary representations to collect hypercube" << std::endl;
    std::vector< std::vector<std::size_t> > binaries;
    binaries = make_binary_list(ndims);

    std::cout << "\n#collecting hypercube corners#" << std::endl;
    for (std::size_t i=0; i<pow(2, ndims); i++) {
      std::vector<std::size_t> a;
      for (std::size_t j=0; j<ndims; j++) {
        a.push_back(floor[j] + binaries[i][j]);
      }
      hypercube.push_back(get_value(a));
      std::cout << get_value(a) << std::endl;
    }
    double result = evaluate_linear(hypercube, weights);
    return result;
  }

  std::vector<size_t> RegularGridInterpolator::find_floor(std::vector<double> target)
  {
    std::vector<size_t> floor;
    for (std::size_t d=0; d<ndims; d+=1) {
      floor.push_back(grid_floor(target[d], d));
      std::cout << target[d] << " is greater than item " << floor[d]
                << " in dim "<< d << ": " << grid[d][floor[d]]<< std::endl;
    }
    return floor;
  }

  std::vector<double> RegularGridInterpolator::set_weights(std::vector<double> target)
  {
    std::vector<double> weights;
    for (std::size_t d=0; d<ndims; d+=1) {
      weights.push_back(get_fraction(target[d], d));
      std::cout << "dim" << d << " fraction = " << weights[d] << std::endl;
    }
    return weights;
  }

  double RegularGridInterpolator::evaluate_linear(
    std::vector<double> hypercube,
    std::vector<double> weights)
  {
    // collapse iteratively from n-dim hypercube to a line.
    std::cout << "\n#starting interpolation#" << std::endl;
    for (std::size_t d=ndims-1; d>0; d--) {
        std::cout << "\nfor dim" << d << ", with frac = " << weights[d] << std::endl;
        hypercube = collapse_dimension(hypercube, weights[d]);
    }

    // interpolate final dimension
    double result = interpolate(weights[0], hypercube[0], hypercube[1]);
    std::cout << "\nfor dim0, with frac = " << weights[0] << std::endl;
    std::cout << hypercube[0] << " & " << hypercube[1] << " => " << result << std::endl;
    return result;
  }

  double RegularGridInterpolator::get_value(std::vector<size_t> x)
  {
    std::size_t index = x[0];
    if (x.size() < 2) {
      return values[index];
    }
    else {
      for (std::size_t j=1; j<x.size(); j++) {
        std::size_t p = x[j];
        for (std::size_t k=0; k<j; k++) {
          p *= dim_lengths[k];
        }
        index += p;
      }
      return values[index];
    }
  }

  // TODO: do something smarter if on the grid point
  std::size_t RegularGridInterpolator::grid_floor(double x, std::size_t dim)
  {
    std::vector<double>::iterator upper;
    upper = std::upper_bound(grid[dim].begin(), grid[dim].end(), x);
    std::size_t floor = upper - grid[dim].begin() - 1;
    return floor;
  }

  double RegularGridInterpolator::get_fraction(double x, std::size_t dim)
  {
    std::size_t floor = RegularGridInterpolator::grid_floor(x, dim);
    double frac = (x - grid[dim][floor]) / (grid[dim][floor+1] -grid[dim][floor]);
    return frac;
  }

}
