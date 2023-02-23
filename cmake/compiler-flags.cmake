add_library(btwxt_interface_library INTERFACE)

target_compile_options(btwxt_interface_library INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>: # Visual C++ (VS 2013)
    /W4   # Warning level (default is W3)
    #/WX  # Turn warnings into errors
  >

  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>: # GCC and Clang
    -Wall       # Turn on all warnings
    -Wextra     # Turn on extra warnings
    -Wpedantic  # Turn on warning not covered in Wall and Wextra
    #-Werror    # Turn warnings into errors
    -fpic       # Position Independent Code
  >
)

