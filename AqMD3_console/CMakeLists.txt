# CMakeList.txt : CMake project for FalkorAcquisition, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project (AqMD3_console)

find_package(cppzmq CONFIG REQUIRED) 
find_package(protobuf CONFIG REQUIRED)
find_package(Snappy CONFIG REQUIRED)

include_directories("C:/Program Files/IVI Foundation/IVI/Include")
link_directories("C:/Program Files/IVI Foundation/IVI/Lib_x64/msc")

# Add source to this project's executable.
add_executable (AqMD3_console 
	src/aqmd3_console.cpp
	src/digitizer.cpp
	src/server.cpp
	src/acquisitionbufferpool.cpp
	src/acquisitionbuffer.cpp
	src/streamingcontext.cpp
	src/server.cpp
	include/digitizer.h
	include/server.h
	include/acquisitionbufferpool.h
	include/acquisitionbuffer.h
	include/streamingcontext.h
	Include/server.h
)
target_link_libraries(AqMD3_console PRIVATE cppzmq cppzmq-static)
target_link_libraries(AqMD3_console PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
target_link_libraries(AqMD3_console PRIVATE AqMD3)
target_link_libraries(AqMD3_console PRIVATE Snappy::snappy)

target_link_libraries ( AqMD3_console PUBLIC i::uimf)