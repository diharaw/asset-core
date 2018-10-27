cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE TERMINUS_SOURCE *.cpp
						  		  *.h
						  		  *.c
						  		  ${PROJECT_SOURCE_DIR}/include/*.h)

add_library(cmft ${TERMINUS_SOURCE})

set_target_properties( cmft
    				   PROPERTIES
    				   ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/cmft"
    				   LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib/cmft"
    				   RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/" )
