project(libgg C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/x64dbg_patch_to_cpp_header.cmake)
set(generated_files_dir ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(patch_src ${CMAKE_CURRENT_LIST_DIR}/patches)
set(patch_dest ${generated_files_dir}/patches)
file(MAKE_DIRECTORY ${patch_dest})
x64dbg_patch_to_cpp_header(
    ${patch_src}/GGXXACPR_Win_Steamless.config-in-game-dir.1337
    ${patch_dest}/config_in_game_dir.h
    "g_patch_config_in_game_dir"
)
x64dbg_patch_to_cpp_header(
    ${patch_src}/GGXXACPR_Win_Steamless.multiple-instances.1337
    ${patch_dest}/multiple_instances.h
    "g_patch_multiple_instances"
)
x64dbg_patch_to_cpp_header(
    ${patch_src}/GGXXACPR_Win_Steamless.open-DisplayConfig.cfg-in-shared-mode.1337
    ${patch_dest}/display_cfg_shared_mode.h
    "g_patch_display_cfg_shared_mode"
)
x64dbg_patch_to_cpp_header(
    ${patch_src}/GGXXACPR_Win_Steamless.predictable-background-io-timing.1337
    ${patch_dest}/predictable_background_io.h
    "g_patch_predictable_background_io"
)
x64dbg_patch_to_cpp_header(
    ${patch_src}/GGXXACPR_Win_Steamless.load_libgg.dll.1337
    ${patch_dest}/load_libgg_dll.h
    "g_patch_load_libgg_dll"
)

add_library(libgg_utils STATIC
    src/utils/attach_console.cpp
    src/utils/binary_patch.cpp
    src/utils/fiber_mgmt.cpp
    src/utils/iat_patch.cpp
    src/utils/memory_protection.cpp
    src/utils/ui.cpp
)
target_link_libraries(libgg_utils PUBLIC dbghelp)
target_include_directories(libgg_utils PUBLIC include)

add_library(libgg SHARED
    src/libgg/game.cpp
    src/libgg/game_state.cpp
    src/libgg/game_state_debug.cpp
    src/libgg/ggpo.cpp
    src/libgg/libgg.cpp
    src/libgg/patches.cpp
    src/libgg/recorder.cpp
    src/libgg/skip_intro.cpp
    src/libgg/sound_fix.cpp
    src/libgg/training_mode_ex.cpp
    src/libgg/command_line.cpp
    src/libgg/configuration.cpp
    src/libgg/print_state.cpp
    src/libgg/unattended.cpp
    src/libgg/libgg.rc
)
target_link_libraries(libgg GGPO libgg_utils libgg_utils Shlwapi)
target_compile_definitions(libgg PRIVATE $<$<CONFIG:Debug>:LIBGG_LOG_FILE>)
target_include_directories(libgg
    PUBLIC include
    PRIVATE ${generated_files_dir}
)

add_executable(libgg_patch WIN32
    src/patch/main.cpp
    src/patch/app.manifest
    src/patch/app.rc
)
target_link_libraries(libgg_patch libgg_utils steamless_lib Comctl32.lib)
target_include_directories(libgg_patch
    PRIVATE ${generated_files_dir}
)
add_dependencies(libgg_patch libgg)
set_source_files_properties(src/patch/app.rc PROPERTIES INCLUDE_DIRECTORIES $<TARGET_FILE_DIR:libgg>)
set_source_files_properties(src/patch/app.rc PROPERTIES OBJECT_DEPENDS libgg.dll)

add_executable(libgg_patch_dev src/patch/main_dev.cpp)
target_link_libraries(libgg_patch_dev libgg_utils steamless_lib)
target_include_directories(libgg_patch_dev
    PRIVATE ${generated_files_dir}
)

add_subdirectory(test)
