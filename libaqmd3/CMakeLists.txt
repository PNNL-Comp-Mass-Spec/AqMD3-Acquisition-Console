# CMakeList.txt : CMake project for FalkorAcquisition, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.12)

project (libaqmd3)

add_library (libaqmd3
	src/digitizer.cpp
	src/acquisitionbuffer.cpp
	src/streamingcontext.cpp
	src/acquireddata.cpp
	src/cstzs1context.cpp
	src/cstcontext.cpp
	src/sa220.cpp
	include/libaqmd3/digitizer.h
	include/libaqmd3/acquisitionbuffer.h
	include/libaqmd3/streamingcontext.h
	include/libaqmd3/cstzs1context.h
	include/libaqmd3/cstcontext.h
	include/libaqmd3/acquireddata.h
	include/libaqmd3/sa220.h
)
add_library(i::digi ALIAS libaqmd3)

include_directories("C:/Program Files/IVI Foundation/IVI/Include")
include_directories("C:/Program Files/IVI Foundation/VISA/Win64/Include")
find_library(AQMD3_LIBRARY AqMD3 HINTS "C:/Program Files/IVI Foundation/IVI/Lib_x64/*")
target_link_libraries(libaqmd3 PRIVATE ${AQMD3_LIBRARY})

# Required for "acquireddata" class. Should be refactored so that this lib is not 
# dependant on the UIMF lib
target_link_libraries(libaqmd3 PUBLIC i::uimf)

target_include_directories ( libaqmd3
	PUBLIC ${PROJECT_SOURCE_DIR}/include
)
