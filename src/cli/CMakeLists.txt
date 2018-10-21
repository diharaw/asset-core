cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")

file(GLOB_RECURSE AST_CLI_SOURCE ${CMAKE_SOURCE_DIR}/src/cli/*.cpp
								 ${CMAKE_SOURCE_DIR}/src/cli/*.h)

add_executable(AssetCoreCLI ${AST_CLI_SOURCE})

target_link_libraries(AssetCoreCLI AssetCoreOffline)