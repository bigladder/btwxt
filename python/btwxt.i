#ifndef BTWXT_I
#define BTWXT_I

%begin %{
  // ... code in begin section ...
%}

%runtime %{
  // ... code in runtime section ...
%}

%header %{
  // ... code in header section ...
%}

%wrapper %{
  // ... code in wrapper section ...
%}

%init %{
  // ... code in init section ...
%}

%module btwxt

%include <stl.i>

%template(DoubleVector) std::vector<double>;
%template(DoubleVectorVector) std::vector<std::vector<double>>;
%template(DoublePair) std::pair<double, double>;

%{
  #include <btwxt/grid-axis.h>
  #include <btwxt/grid-point-data.h>
  #include <btwxt/regular-grid-interpolator.h>
%}

%template(GridAxisVector) std::vector<Btwxt::GridAxis>;
%template(GridPointDataSetVector) std::vector<Btwxt::GridPointDataSet>;
%template(TargetBoundsStatusVector) std::vector<Btwxt::TargetBoundsStatus>;

%include <btwxt/grid-axis.h>
%include <btwxt/grid-point-data.h>
%include <btwxt/regular-grid-interpolator.h>

#endif // BTWXT_I
