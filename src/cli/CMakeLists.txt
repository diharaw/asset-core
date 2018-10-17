cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

IF(APPLE)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
ENDIF()

file(GLOB_RECURSE AST_CLI_SOURCE src/cli/*.cpp
						  	 	 src/cli/*.h)

add_library(AssetCoreCLI ${AST_CLI_SOURCE})

target_link_libraries(AssetCoreCLI AssetCoreOffline)