# add_gg_test(NAME [ARGS args...] [TIMEOUT seconds] [DISABLED]
# NAME: path to test replay is based on test name: <SRC_DIR>/replays/<NAME>.ggr
# ARGS: arguments for gg.exe / GGXXACPR_Win.exe. For full list of supported arguments,
#       run gg.exe --help.
function(add_gg_test NAME)
    cmake_parse_arguments(
        PARSE_ARGV 1 "" "DISABLED" "TIMEOUT" "ARGS"
    )

    if(_DISABLED)
        return()
    endif()

    set(test_name gg_${NAME})

    if(LIBGG_TEST_UPDATE_STATE_CHECKSUM)
        # Game state has been extended, thus exactly the same frame
        # will produce a different state checksum.
        # Checksum values stored in replay files need to be updated.
        set(_ARGS --record)
    endif()
    if(LIBGG_TEST_NOGRAPHICS)
        list(APPEND _ARGS --nographics)
    endif()
    if(LIBGG_TEST_NOSOUND)
        list(APPEND _ARGS --nosound)
    endif()
    list(APPEND _ARGS --noinput)

    add_test(NAME ${test_name}
        COMMAND gg --replay ${CMAKE_SOURCE_DIR}/replays/${NAME}.ggr ${_ARGS}
    )

    set(timeout 60)
    if (_TIMEOUT)
        set(timeout ${_TIMEOUT})
    endif()
    set_tests_properties(${test_name} PROPERTIES
        TIMEOUT ${timeout}
    )

    if(LIBGG_TEST_RUN_SERIAL)
        set_tests_properties(${test_name} PROPERTIES
            RUN_SERIAL ON
        )
    endif()
endfunction()
