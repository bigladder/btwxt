

// Standard
#include<iostream>

// example: https://github.com/Crascit/DownloadProject/blob/master/example.cpp
// example2: https://github.com/bigladder/kiva/blob/develop/test/unit/foundation.unit.cpp

// btwxt
#include <longtable.h>



double interpolate(double t, double a0, double a1) {
  // general linear interpolation in one dimension
  return t*a1 + (1-t)*a0;
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


std::size_t pow(std::size_t x, std::size_t power) {
  // raise x to a power (both must be size_t)
  std::size_t result = x;
  for (std::size_t i=1; i<power; i++)
  {
      result = result*x;
  }
  return result;
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


int main(int argc, char **argv)
{

    std::cout << "#### Scratch Work ####" << std::endl;

    Interp::LongTable my_longtable;

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
    // std::vector<double> third = {1, 2, 3};
    // std::vector< std::vector<double> > grid = {first, second, third};
    // double values[] = {
    //   24421.69383, 25997.3589, 28392.31868, 29655.22876, 31094.97495, 33988.3473,
    //   22779.73113, 24352.1562, 26742.74198, 28003.546, 29441.02425, 32330.1846,
    //   21147.21662, 22716.4017, 25102.61348, 26361.31143, 27796.52175, 30681.4701,
    //   19794.00525, 21360.49033, 23743.0571, 25000, 26433.32038, 29314.75872,
    //   19524.15032, 21090.0954, 23471.93318, 24728.52506, 26161.46745, 29042.2038,
    //   18178.81244, 19742.05753, 22120.2503, 23375.08713, 24806.13958, 27683.36592,
    //   16810.36004, 18370.84513, 20745.3119, 21998.35468, 23427.47518, 26301.11353,
    //   29421.69383, 30997.3589, 33392.31868, 34655.22876, 36094.97495, 38988.3473,
    //   27779.73113, 29352.1562, 31742.74198, 33003.546, 34441.02425, 37330.1846,
    //   26147.21662, 27716.4017, 30102.61348, 31361.31143, 32796.52175, 35681.4701,
    //   24794.00525, 26360.49033, 28743.0571, 30000, 31433.32038, 34314.75872,
    //   24524.15032, 26090.0954, 28471.93318, 29728.52506, 31161.46745, 34042.2038,
    //   23178.81244, 24742.05753, 27120.2503, 28375.08713, 29806.13958, 32683.36592,
    //   21810.36004, 23370.84513, 25745.3119, 26998.35468, 28427.47518, 31301.11353,
    //   31421.69383, 32997.3589, 35392.31868, 36655.22876, 38094.97495, 40988.3473,
    //   29779.73113, 31352.1562, 33742.74198, 35003.546, 36441.02425, 39330.1846,
    //   28147.21662, 29716.4017, 32102.61348, 33361.31143, 34796.52175, 37681.4701,
    //   26794.00525, 28360.49033, 30743.0571, 32000, 33433.32038, 36314.75872,
    //   26524.15032, 28090.0954, 30471.93318, 31728.52506, 33161.46745, 36042.2038,
    //   25178.81244, 26742.05753, 29120.2503, 30375.08713, 31806.13958, 34683.36592,
    //   23810.36004, 25370.84513, 27745.3119, 28998.35468, 30427.47518, 33301.11353
    //   };
    my_longtable = Interp::LongTable(grid, values);



    std::size_t ndims = my_longtable.get_ndims();
    std::cout << "we have a " << ndims << "D table with " << my_longtable.get_nvalues() << " values." << std::endl;

    std::vector<size_t> dim_lengths = my_longtable.get_dim_lengths();
    std::cout << "the table dimensions are:" << std::endl;
    for (auto dim : dim_lengths) {
      std::cout << dim << std::endl;
    }

    std::cout << "\nthe inputs we seek to interpolate, and how they fit on the grid: " << std::endl;
    double target[] = {20, 31, 1.5};
    std::vector<size_t> floors;
    for (std::size_t d=0; d<ndims; d+=1) {
      floors.push_back(my_longtable.grid_floor(target[d], d));
      std::cout << target[d] << " is greater than item " << floors[d]
                << " in dim "<< d << ": " << grid[d][floors[d]]<< std::endl;
    }

    // get fractions of the grid-space crossing on each axis
    std::cout << "\nhow far in the hypercube in each dimension: " << std::endl;
    std::vector<double> fracs;
    // double fracs[2];
    for (std::size_t d=0; d<ndims; d+=1) {
      fracs.push_back(my_longtable.get_fraction(target[d], d));
      // fracs[d] = (target[d] - grid[d][floors[d]]) / (grid[d][floors[d]+1] - grid[d][floors[d]]);
      std::cout << "dim" << d << " fraction = " << fracs[d] << std::endl;
    }



    // collect all of the points in the interpolation hypercube
    std::vector<double> hypercube;

    std::cout << "\nwe use binary representations to collect hypercube" << std::endl;
    std::vector< std::vector<std::size_t> > binaries;
    binaries = make_binary_list(ndims);

    std::cout << "\n#collecting hypercube corners#" << std::endl;
    for (std::size_t i=0; i<pow(2, ndims); i++) {
      std::vector<std::size_t> a;
      for (std::size_t j=0; j<ndims; j++) {
        a.push_back(floors[j] + binaries[i][j]);
      }
      hypercube.push_back(my_longtable.get_value(a));
      std::cout << my_longtable.get_value(a) << std::endl;
    }

    // collapse iteratively from n-dim hypercube to a line.
    std::cout << "\n#starting interpolation#" << std::endl;
    for (std::size_t d=ndims-1; d>0; d--) {
        std::cout << "\nfor dim" << d << ", with frac = " << fracs[d] << std::endl;
        hypercube = collapse_dimension(hypercube, fracs[d]);
    }

    // interpolate final dimension
    double result = interpolate(fracs[0], hypercube[0], hypercube[1]);
    std::cout << "\nfor dim0, with frac = " << fracs[0] << std::endl;
    std::cout << hypercube[0] << " & " << hypercube[1] << " => " << result << std::endl;

    return 0;
};
