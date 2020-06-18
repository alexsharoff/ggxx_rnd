set(_X64DBG_PATCH_TEMPLATE "${CMAKE_CURRENT_LIST_DIR}/patch.h.in")

# Convert .1337 patch into a C++ header
function(x64dbg_patch_to_cpp_header IN_1337 OUT_HEADER VAR_NAME)
    file(STRINGS ${IN_1337} lines)
    set(FILENAME)
    set(REPLACEMENTS)
    foreach(line ${lines})
        if(NOT FILENAME)
            string(SUBSTRING ${line} 0 1 test)
            if(NOT test STREQUAL ">")
                message(FATAL_ERROR "Invalid .1337 file")
            endif()
            string(SUBSTRING ${line} 1 -1 FILENAME)
        else()
            string(
                REGEX REPLACE
                "^0+([1-9A-F]+[0-9A-F]*):([0-9A-F]+)->([0-9A-F]+)$"
                "{0x\\1, 0x\\2, 0x\\3}"
                triple
                ${line}
            )
            if(NOT triple)
                message(FATAL_ERROR "Unable to parse line: ${line}")
            endif()
            if(REPLACEMENTS)
                string(APPEND REPLACEMENTS ",")
            endif()
            string(APPEND REPLACEMENTS "\n        ${triple}")
        endif()
    endforeach()

    if(NOT FILENAME OR NOT REPLACEMENTS)
        message(FATAL_ERROR "Invalid or empty .1337 file")
    endif()

    configure_file("${_X64DBG_PATCH_TEMPLATE}" "${OUT_HEADER}" @ONLY)
endfunction()
