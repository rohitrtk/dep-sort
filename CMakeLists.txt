cmake_minimum_required(VERSION 3.25)

project(dep-sort LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    URL_HASH SHA256=42f6e95cad6ec532fd372391373363b62a14af6d771056dbfc86160e6dfff7aa
)
FetchContent_MakeAvailable(nlohmann_json)

file(GLOB_RECURSE dep_sort_sources CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(dep-sort ${dep_sort_sources})
target_include_directories(dep-sort PRIVATE src)
target_compile_features(dep-sort PRIVATE cxx_std_23)
set_target_properties(dep-sort PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(dep-sort PRIVATE nlohmann_json::nlohmann_json)
