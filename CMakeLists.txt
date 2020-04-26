cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

project(libgg C CXX)

# TODO: automate
# Copy to build dir, run steamless, apply patches
set(GGXXACPR_DIR "" CACHE PATH "")

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Only 32-bit build is supported")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Link static MSVCRT for all targets
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_compile_options($<$<NOT:$<CONFIG:Debug>>:/GL> /W4 /WX /Wv:18)
add_link_options($<$<NOT:$<CONFIG:Debug>>:/LTCG>)

add_subdirectory(ggpo)

add_compile_definitions(
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

include_directories(include)

add_library(libgg.static STATIC
    src/patch_iat.cpp
    src/fiber_mgmt.cpp
)
target_link_libraries(libgg.static PUBLIC dbghelp)

add_library(libgg SHARED
    src/game.cpp
    src/game_state.cpp
    src/ggpo.cpp
    src/libgg.cpp
    src/patches.cpp
    src/recorder.cpp
    src/skip_intro.cpp
    src/sound_fix.cpp
    src/training_mode_ex.cpp
    src/command_line.cpp
    src/configuration.cpp
    src/util.cpp
    src/rollback_test.cpp
    src/print_state.cpp
)
target_link_libraries(libgg GGPO libgg.static Shlwapi)
target_compile_definitions(libgg PRIVATE $<$<CONFIG:Debug>:LIBGG_LOG_ENABLE>)

add_subdirectory(src/steam_api_nop)

enable_testing()

add_executable(test_mini_reflection test/mini_reflection.cpp)
add_executable(test_fiber_mgmt test/fiber_mgmt.cpp)
target_link_libraries(test_fiber_mgmt libgg.static)
#add_executable(test_memory_dump test/memory_dump.cpp)
#add_executable(test_diff test/diff.cpp)
#add_executable(test_binary_serializer test/binary_serializer.cpp)
#add_executable(test_pretty_print test/pretty_print.cpp)

add_executable(gg IMPORTED)
set_target_properties(gg PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/gg.exe)

add_test(NAME mini_reflection COMMAND test_mini_reflection)
add_test(NAME fiber_mgmt COMMAND test_fiber_mgmt)
#add_test(NAME memory_dump COMMAND test_memory_dump)
#add_test(NAME diff COMMAND test_diff)
#add_test(NAME binary_serializer COMMAND test_binary_serializer)
#add_test(NAME pretty_print COMMAND test_pretty_print)

include(add_gg_test)

# Enable once [!] after game state has been extended and all autotests started to fail
set(LIBGG_TEST_UPDATE_STATE_CHECKSUM OFF)
# Disable graphics and sound for autotests
# Speeds things up greatly
set(LIBGG_TEST_NOGRAPHICS ON)
set(LIBGG_TEST_NOSOUND OFF)
# Don't run tests in parallel
set(LIBGG_TEST_RUN_SERIAL OFF)

add_gg_test(vs2p_fafnir_tyrant_rave ARGS --gamemode network --checkstate --synctest 8
    DISABLED # TODO: support BFBT and related fibers
)
add_gg_test(vs2p_kliff_throw ARGS --gamemode network --checkstate --synctest 1
    DISABLED # TODO: support BFBT and related fibers
)
add_gg_test(vs2p_faust_taunt ARGS
    --checkstate
    --rollback 1400 1050
    --rollback 1460 1050
)
add_gg_test(training_kliff_taunt ARGS
    --checkstate
    --rollback 840 691
    --rollback 1289 840
    --rollback 1300 1288
)
add_gg_test(charselect_screen ARGS
    --gamemode network --checkstate --synctest 1
)
add_gg_test(charselect_screen_random_char ARGS
    --gamemode network --checkstate --synctest 1
)
add_gg_test(charselect_screen_random_stage ARGS
    --gamemode network --checkstate --synctest 1
)
