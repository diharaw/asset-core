cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -stdlib=libc++")
endif()

file(GLOB_RECURSE SH_PROJECT_SOURCE ${PROJECT_SOURCE_DIR}/src/sh_project/*.cpp
								  	${PROJECT_SOURCE_DIR}/src/sh_project/*.h)

add_executable(sh_project ${SH_PROJECT_SOURCE})

set_property(TARGET sh_project PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$(Configuration)")

target_link_libraries(sh_project AssetCoreImporter)
target_link_libraries(sh_project AssetCoreExporter)
target_link_libraries(sh_project AssetCoreLoader)
