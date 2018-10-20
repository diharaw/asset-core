cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

IF(APPLE)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
ENDIF()

file(GLOB_RECURSE AST_RUNTIME_SOURCE ${CMAKE_SOURCE_DIR}/src/runtime/*.cpp
									 ${CMAKE_SOURCE_DIR}/include/runtime/*.h)

add_library(AssetCoreRuntime ${AST_RUNTIME_SOURCE})

target_link_libraries(AssetCoreRuntime AssetCoreCommon)