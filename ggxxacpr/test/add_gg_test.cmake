if(NOT DEFINED GG_TEST_EXECUTABLE)
    message(FATAL_ERROR "GG_TEST_EXECUTABLE must be defined")
endif()
if(NOT DEFINED GG_TEST_TIMEOUT)
    set(GG_TEST_TIMEOUT 60)
endif()
if(NOT DEFINED GG_TEST_RUN_SERIAL)
    set(GG_TEST_RUN_SERIAL OFF)
endif()
if(NOT DEFINED GG_TEST_UNATTENDED)
    set(GG_TEST_UNATTENDED ON)
endif()
if(NOT DEFINED GG_TEST_REPLAY_DIR)
    set(GG_TEST_REPLAY_DIR "${CMAKE_CURRENT_LIST_DIR}/replays")
endif()

# add_gg_test(REPLAY path [NAME name ] [TIMEOUT seconds] [DISABLED] [RUN_SERIAL ON/OFF] [ARGS args...]
# REPLAY path relative to GG_TEST_REPLAY_DIR (${CMAKE_SOURCE_DIR}/test/replays)
# NAME: test name
# ARGS: arguments for gg.exe / GGXXACPR_Win.exe. For full list of supported arguments,
#       run gg.exe /help.
# DISABLED: skip test
# TIMEOUT: test timeout in seconds. Default: GG_TEST_TIMEOUT (60)
# RUN_SERIAL: don't run other test simultaneously with this one. Default: GG_TEST_RUN_SERIAL (OFF)
function(add_gg_test)
    cmake_parse_arguments(
        PARSE_ARGV 0 "" "DISABLED" "NAME;REPLAY;TIMEOUT;RUN_SERIAL" "ARGS"
    )

    if(_DISABLED)
        return()
    endif()

    if(NOT _REPLAY)
        message(FATAL_ERROR "REPLAY is not specified or invalid")
    endif()

    if(NOT IS_ABSOLUTE ${_REPLAY})
        set(_REPLAY ${GG_TEST_REPLAY_DIR}/${_REPLAY})
    endif()

    if(NOT EXISTS "${_REPLAY}")
        message(FATAL_ERROR "File does not exist: ${_REPLAY}")
    endif()

    if(NOT _NAME)
        get_filename_component(_NAME ${_REPLAY} NAME_WE)
    endif()

    foreach(var _TIMEOUT _RUN_SERIAL)
        # set default value if unspecified
        if(NOT DEFINED ${var})
            set(${var} ${GG_TEST${var}})
        endif()
    endforeach()

    if(GG_TEST_UNATTENDED)
        list(APPEND _ARGS /unattended)
    endif()

    add_test(NAME ${_NAME} COMMAND ${GG_TEST_EXECUTABLE} ${_REPLAY} ${_ARGS})

    set_tests_properties(${_NAME} PROPERTIES
        TIMEOUT ${_TIMEOUT}
        RUN_SERIAL ${_RUN_SERIAL}
    )
endfunction()
