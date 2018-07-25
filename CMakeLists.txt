cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("AssetConverter")

set(ASSIMP_INCLUDE_DIRS "external/assimp/include" "${CMAKE_CURRENT_BINARY_DIR}/external/assimp/include")
set(JSON_INCLUDE_DIRS "external/json/single_include/nlohmann")
set(GLM_INCLUDE_DIRS "external/glm/glm")
set(CORE_INCLUDE_DIRS "include")

add_subdirectory("external/assimp")

IF(APPLE)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
ENDIF()

# set(CMAKE_PREFIX_PATH "/Users/diharaw/Qt/5.8/clang_64/lib/cmake")

# find_package(Qt5Widgets)

# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories("${ASSIMP_INCLUDE_DIRS}"
					"${JSON_INCLUDE_DIRS}"
					"${GLM_INCLUDE_DIRS}"
					"${CORE_INCLUDE_DIRS}")

# SET(TSM_GUI_SOURCE src/gui_importer/main.cpp src/gui_importer/mainwindow.cpp)
# SET(TSM_GUI_HEADERS src/gui_importer/mainwindow.h)
# SET(TSM_GUI_FORMS src/gui_importer/mainwindow.ui)

# QT5_WRAP_CPP(TSM_GUI_HEADERS_MOC ${TSM_GUI_HEADERS})
# QT5_WRAP_UI(TSM_GUI_FORMS_HEADERS ${TSM_GUI_FORMS})

file(GLOB_RECURSE CORE_SOURCE src/core/*.cpp
						  	 include/*.h)

file(GLOB_RECURSE CLI_SOURCE src/cli/*.cpp
						  	 src/cli/*.h)

add_library(AssetConverterCore ${CORE_SOURCE})
target_link_libraries(AssetConverterCore assimp)

add_executable(AssetConverter ${CLI_SOURCE})
target_link_libraries(AssetConverter AssetConverterCore)
target_link_libraries(AssetConverter assimp)

# add_executable(TSM_GUI ${TSM_GUI_SOURCE})
# target_link_libraries(TSM_GUI TSM_Core)
# target_link_libraries(TSM_GUI Qt5::Widgets)
# target_link_libraries(TSM_GUI assimp)
