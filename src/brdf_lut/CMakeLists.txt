cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
endif()

file(GLOB_RECURSE BRDF_LUT_SOURCE ${PROJECT_SOURCE_DIR}/src/brdf_lut/*.cpp
								  ${PROJECT_SOURCE_DIR}/src/brdf_lut/*.h)

add_executable(brdf_lut ${BRDF_LUT_SOURCE})