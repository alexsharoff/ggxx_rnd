cmake_minimum_required(VERSION 3.15)

project(ggxxrnd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)
# Enable whole program optimization, treat warnings as errors
add_compile_options($<$<NOT:$<CONFIG:Debug>>:/GL> /W4 /WX)
add_link_options($<$<NOT:$<CONFIG:Debug>>:/LTCG>)
# Link static MSVCRT for all targets
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

enable_testing()

add_subdirectory(thirdparty)
add_subdirectory(libgg)
add_subdirectory(ggxxacpr)
