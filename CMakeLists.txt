# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.12)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

project ("AqMD3_console")


# Include sub-projects.
add_subdirectory ("UIMFWriter")
add_subdirectory ("AqMD3_console")
