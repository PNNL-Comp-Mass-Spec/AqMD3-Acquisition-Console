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

include_directories("C:/Program Files/IVI Foundation/VISA/Win64/Include")
link_directories("C:/Program Files/IVI Foundation/VISA/Win64L/lib_x64/msc")

# Add source to this project's executable.
add_executable (AqMD3_console 
	src/aqmd3_console.cpp
	src/digitizer.cpp
	src/server.cpp
	src/acquisitionbuffer.cpp
	src/streamingcontext.cpp
	src/server.cpp
	src/cstzs1context.cpp
	src/cstcontext.cpp
	src/message.pb.cpp
	src/acquireddata.cpp
	src/sa220.cpp
	src/acquirepublisher.cpp
	src/processsubject.cpp
	src/zmqacquireddatasubscriber.cpp
	src/uimfframesubscriber.cpp
	src/uimfframewritersubscriber.cpp
	include/digitizer.h
	include/server.h
	include/acquisitionbuffer.h
	include/streamingcontext.h
	include/server.h
	include/cstzs1context.h
	include/cstcontext.h
	include/message.pb.h
	include/acquireddata.h
	include/sa220.h
	include/framepublisher.h
	include/framesubscriber.h
	include/acquirepublisher.h
	include/acquisitioncontrol.h
	include/processsubject.h
	include/zmqacquireddatasubscriber.h
	include/uimfframesubscriber.h
#	include/diagnostic/rawprintersubscriber.h
#	include/diagnostic/simplepublisher.h
#	include/diagnostic/falsedatapublisher.h
#	include/diagnostic/encodeddatawriter.h
	include/uimfframewritersubscriber.h
	include/definitions.h
)

target_link_libraries(AqMD3_console PRIVATE cppzmq cppzmq-static)
target_link_libraries(AqMD3_console PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)#
target_link_libraries(AqMD3_console PRIVATE Snappy::snappy)

target_link_libraries(AqMD3_console PRIVATE AqMD3)
target_link_libraries(AqMD3_console PRIVATE AqLio)

target_link_libraries (AqMD3_console PUBLIC i::uimf)