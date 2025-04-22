# CMakeList.txt : CMake project for FalkorAcquisition, include source and define
# project specific logic here.
#

project (UIMFWriter)

find_package(SQLiteCpp CONFIG REQUIRED)

add_library (UIMFWriter 
src/uimfwriter.cpp
src/encodedresult.cpp
src/uimfframe.cpp
src/lzf_c.c
src/lzf_d.c
#src/lzf_c_best.c
include/UIMFWriter/encodedresult.h
include/UIMFWriter/uimfwriter.h
include/UIMFWriter/uimfframe.h
include/lzf/lzf.h
include/lzf/lzfP.h
 "include/UIMFWriter/uimfframeparameters.h")
add_library(i::uimf ALIAS UIMFWriter)
    
target_link_libraries(UIMFWriter PRIVATE SQLiteCpp)

target_include_directories ( UIMFWriter
	PUBLIC ${PROJECT_SOURCE_DIR}/include
)
