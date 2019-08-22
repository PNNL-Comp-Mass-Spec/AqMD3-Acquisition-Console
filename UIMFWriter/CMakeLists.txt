# CMakeList.txt : CMake project for FalkorAcquisition, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (UIMFWriter)

find_package(sqlite3 CONFIG REQUIRED)

add_library (UIMFWriter "src/UIMFWriter.cpp")
add_library(i::uimf ALIAS UIMFWriter)

target_link_libraries(UIMFWriter PRIVATE sqlite3)

target_include_directories ( UIMFWriter
	PUBLIC ${PROJECT_SOURCE_DIR}/include
)
