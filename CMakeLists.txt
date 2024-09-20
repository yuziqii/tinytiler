cmake_minimum_required(VERSION 3.10)
project(3dtilespp VERSION 1.0 LANGUAGES CXX)

set(TARGET_NAME osgb2tiles)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${TARGET_NAME} 
    ${SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
)

find_package(osg REQUIRED)
find_package(osgDB REQUIRED)
find_package(osgUtil REQUIRED)

target_include_directories(${TARGET_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_subdirectory(extern)

target_link_libraries(${TARGET_NAME} 
    ${OSG_LIBRARY}
    ${OSGDB_LIBRARY}
    ${OSGUTIL_LIBRARY}
    nlohmann_json::nlohmann_json
    cxxopts::cxxopts
    tinyxml2::tinyxml2
    PROJ::proj
)