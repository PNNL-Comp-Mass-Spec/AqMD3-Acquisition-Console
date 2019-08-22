# CMakeList.txt : CMake project for FalkorAcquisition, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (AqMD3)

find_package(cppzmq CONFIG REQUIRED) 
find_package(protobuf CONFIG REQUIRED)
find_package(Snappy CONFIG REQUIRED)

include_directories("C:/Program Files/IVI Foundation/IVI/Include")
link_directories("C:/Program Files/IVI Foundation/IVI/Lib_x64/msc")

# Add source to this project's executable.
add_executable (AqMD3 
	src/FalkorAcquisition.cpp
	src/digitizer.cpp
	src/server.cpp
	src/acquisitionbufferpool.cpp
	src/acquisitionbuffer.cpp
	src/acquisitioncontext.cpp
	src/server.cpp
	include/digitizer.h
	include/server.h
	include/acquisitionbufferpool.h
	include/acquisitionbuffer.h
	include/acquisitioncontext.h
	Include/server.h
)
target_link_libraries(AqMD3 PRIVATE cppzmq cppzmq-static)
target_link_libraries(AqMD3 PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
target_link_libraries(AqMD3 PRIVATE AqMD3)
target_link_libraries(AqMD3 PRIVATE Snappy::snappy)

target_link_libraries ( AqMD3 PUBLIC i::uimf)