function(add_gg_test NAME)
    cmake_parse_arguments(
        PARSE_ARGV 1 "" "" "TIMEOUT" "ARGS"
    )

    set(test_name gg_${NAME})

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
endfunction()
