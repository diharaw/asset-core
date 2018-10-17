cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

IF(APPLE)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
ENDIF()

file(GLOB_RECURSE AST_OFFLINE_SOURCE ${CMAKE_SOURCE_DIR}/src/offline/*.cpp
									 ${CMAKE_SOURCE_DIR}/include/offline/*.h)

add_library(AssetCoreOffline ${AST_OFFLINE_SOURCE})

target_link_libraries(AssetCoreOffline assimp)