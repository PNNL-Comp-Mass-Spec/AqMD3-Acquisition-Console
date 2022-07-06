# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.12)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")
set (VCPKG_FEATURE_FLAGS "versions")
project ("AqMD3_console")


# Include sub-projects.
add_subdirectory ("UIMFWriter")
add_subdirectory ("libaqmd3")
add_subdirectory ("AqMD3_console")

target_compile_options(AqMD3_console PRIVATE "$<$<CONFIG:Release>:/Zi>")
target_compile_options(AqMD3_console PRIVATE "$<$<CONFIG:Release>:/EHa>")
target_link_options(AqMD3_console PRIVATE "$<$<CONFIG:Release>:/DEBUG>")
target_link_options(AqMD3_console PRIVATE "$<$<CONFIG:Release>:/OPT:REF>")
target_link_options(AqMD3_console PRIVATE "$<$<CONFIG:Release>:/OPT:ICF>")
