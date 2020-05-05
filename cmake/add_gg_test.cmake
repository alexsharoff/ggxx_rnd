if(NOT DEFINED LIBGG_TEST_TIMEOUT)
    set(LIBGG_TEST_TIMEOUT 60)
endif()
if(NOT DEFINED LIBGG_TEST_RUN_SERIAL)
    set(LIBGG_TEST_RUN_SERIAL ON)
endif()
if(NOT DEFINED LIBGG_TEST_NOWINDOW)
    set(LIBGG_TEST_NOWINDOW OFF)
endif()
if(NOT DEFINED LIBGG_TEST_REPLAY_DIR)
    set(LIBGG_TEST_REPLAY_DIR "${CMAKE_SOURCE_DIR}/test/replays")
endif()

# add_gg_test(REPLAY path [NAME name ] [TIMEOUT seconds] [DISABLED] [RUN_SERIAL ON/OFF] [ARGS args...]
# REPLAY path relative to LIBGG_TEST_REPLAY_DIR (${CMAKE_SOURCE_DIR}/test/replays)
# NAME: test name
# ARGS: arguments for gg.exe / GGXXACPR_Win.exe. For full list of supported arguments,
#       run gg.exe /help.
# DISABLED: skip test
# TIMEOUT: test timeout in seconds. Default: LIBGG_TEST_TIMEOUT (60)
# RUN_SERIAL: don't run other test simultaneously with this one. Default: LIBGG_TEST_RUN_SERIAL (OFF)
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
        set(_REPLAY ${LIBGG_TEST_REPLAY_DIR}/${_REPLAY})
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
            set(${var} ${LIBGG_TEST${var}})
        endif()
    endforeach()

    if(LIBGG_TEST_NOWINDOW)
        list(APPEND _ARGS /nowindow)
    endif()

    set(command gg ${_REPLAY} ${_ARGS})
    string(REPLACE ";" " " command_str "${command}")
    message(STATUS "${_NAME}: ${command_str}")
    add_test(NAME ${_NAME} COMMAND ${command})

    set_tests_properties(${_NAME} PROPERTIES
        TIMEOUT ${_TIMEOUT}
        RUN_SERIAL ${_RUN_SERIAL}
    )
endfunction()
