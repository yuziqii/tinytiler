set(tinyxml2_SHARED_LIBS OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(cxxopts)
add_subdirectory(tinyxml2)
add_subdirectory(nlohmann-json)

set(BUILD_TESTING OFF)
set(PROJ_DB_CACHE_DIR "${CMAKE_BINARY_DIR}/proj")
add_subdirectory(proj)

add_library(earcut INTERFACE)
target_include_directories(earcut SYSTEM INTERFACE earcut/include)
set_target_properties(earcut PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/earcut/include/mapbox/earcut.hpp")
