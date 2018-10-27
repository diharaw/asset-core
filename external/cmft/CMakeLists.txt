cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project("cmft")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/external")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/external")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

IF(APPLE)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000")
ENDIF()

set(CL_INCLUDE_DIRS  "${PROJECT_SOURCE_DIR}/dependency/CL")
set(DEP_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/dependency")
set(CMFT_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/include")


include_directories("${CL_INCLUDE_DIRS}" 
					"${DEP_INCLUDE_DIRS}"
					"${CMFT_INCLUDE_DIRS}")

add_subdirectory(src/cmft)
