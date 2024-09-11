add_subdirectory(cxxopts)
add_subdirectory(tinyxml2)
add_subdirectory(nlohmann-json)

add_library(earcut INTERFACE)
target_include_directories(earcut SYSTEM INTERFACE earcut/include)
set_target_properties(earcut PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/earcut/include/mapbox/earcut.hpp")
