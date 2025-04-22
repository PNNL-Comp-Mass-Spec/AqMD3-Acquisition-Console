# CMakeList.txt : CMake project for FalkorAcquisition, include source and define
# project specific logic here.
#

project (AqMD3_console)

find_path(PICOSHA2_INCLUDE_DIRS "picosha2.h")

#set(BOOST_STACKTRACE_USE_WINDBG)

find_package(cppzmq CONFIG REQUIRED) 
find_package(protobuf CONFIG REQUIRED)
find_package(Snappy CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
#find_package(Boost REQUIRED COMPONENTS stacktrace_windbg exception)

#if(Boost_FOUND)
#    include_directories(${Boost_INCLUDE_DIRS})
#    link_directories(${Boost_LIBRARY_DIRS})
#endif()

include_directories("C:/Program Files/IVI Foundation/IVI/Include")
link_directories("C:/Program Files/IVI Foundation/IVI/Lib_x64/msc")

include_directories("C:/Program Files/IVI Foundation/VISA/Win64/Include")
link_directories("C:/Program Files/IVI Foundation/VISA/Win64L/lib_x64/msc")

# Add source to this project's executable.
add_executable (AqMD3_console 
	src/aqmd3_console.cpp
	src/server.cpp
	src/server.cpp
	src/message.pb.cpp
	src/acquirepublisher.cpp
	src/processsubject.cpp
	src/zmqacquireddatasubscriber.cpp
	src/uimfframewritersubscriber.cpp
	src/uimfacquisitionrecord.cpp
	include/server.h
	include/server.h
	include/message.pb.h
	include/publisher.h
	include/subscriber.h
	include/acquirepublisher.h
	include/acquisitioncontrol.h
	include/processsubject.h
	include/zmqacquireddatasubscriber.h
	include/diagnostic/datageneratorcontext.h
	include/uimfframewritersubscriber.h
	include/definitions.h
	include/util/timehelpers.h
	include/util/config.h  
	include/uimfacquisitionrecord.h 
	include/util/uimfhelpers.h)
target_include_directories(AqMD3_console PRIVATE ${PICOSHA2_INCLUDE_DIRS})

target_link_libraries(AqMD3_console PRIVATE cppzmq cppzmq-static)
target_link_libraries(AqMD3_console PRIVATE protobuf::libprotoc protobuf::libprotobuf protobuf::libprotobuf-lite)
target_link_libraries(AqMD3_console PRIVATE Snappy::snappy)

#target_link_libraries(AqMD3_console PRIVATE AqMD3)
target_link_libraries(AqMD3_console PRIVATE AqLio)

target_link_libraries (AqMD3_console PUBLIC i::uimf)
target_link_libraries (AqMD3_console PUBLIC i::digi)

target_link_libraries(AqMD3_console PRIVATE spdlog::spdlog spdlog::spdlog_header_only)
#message(STATUS "Boost: ${Boost_LIBRARIES}")
#target_link_libraries(AqMD3_console PRIVATE ${Boost_LIBRARIES})
#target_link_libraries(AqMD3_console PRIVATE Boost::boost Boost::exception Boost::stacktrace)

add_custom_command(TARGET AqMD3_console PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/config.txt $<TARGET_FILE_DIR:AqMD3_console>
)
add_custom_command(TARGET AqMD3_console POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/include/app.h $<TARGET_FILE_DIR:AqMD3_console>
)
