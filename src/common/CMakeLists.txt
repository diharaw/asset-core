cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE AST_COMMON_SOURCE ${PROJECT_SOURCE_DIR}/src/common/*.cpp
									${PROJECT_SOURCE_DIR}/include/common/*.h)

add_library(AssetCoreCommon ${AST_COMMON_SOURCE})