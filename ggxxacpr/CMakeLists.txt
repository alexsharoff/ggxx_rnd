set(GGXXACPR_DIR
    "C:/Program Files (x86)/Steam/steamapps/common/Guilty Gear XX Accent Core Plus R"
    CACHE PATH
    "GUILTY GEAR XX ACCENT CORE PLUS R (Steam) Installation Directory"
)

if(GGXXACPR_DIR)
    message(STATUS "GGXXACPR_DIR: ${GGXXACPR_DIR}")
endif()

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Only 32-bit build is supported")
endif()

if(EXISTS ${GGXXACPR_DIR}/GGXXACPR_Win.exe)
    set(ggxxacpr_out_dir ${CMAKE_CURRENT_BINARY_DIR}/bin)
    if(NOT CMAKE_CFG_INTDIR STREQUAL ".")
        set(ggxxacpr_dev ${ggxxacpr_out_dir}/${CMAKE_CFG_INTDIR}/gg.exe)
        # add_test(COMMAND) doesn't support CMAKE_CFG_INTDIR
        set(GG_TEST_EXECUTABLE ${ggxxacpr_out_dir}/$<CONFIG>/gg.exe)
    else()
        set(ggxxacpr_dev ${ggxxacpr_out_dir}/gg.exe)
        set(GG_TEST_EXECUTABLE ${ggxxacpr_dev})
    endif()

    set_target_properties(libgg steam_api PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ggxxacpr_out_dir})

    add_custom_command(OUTPUT ${ggxxacpr_dev}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${ggxxacpr_out_dir}/${CMAKE_CFG_INTDIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${GGXXACPR_DIR}/Resource ${ggxxacpr_out_dir}/${CMAKE_CFG_INTDIR}/Resource
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/test/configs/Savedata.dat ${ggxxacpr_out_dir}/${CMAKE_CFG_INTDIR}/Savedata.dat
        COMMAND libgg_patch_dev ${GGXXACPR_DIR}/GGXXACPR_Win.exe ${ggxxacpr_dev}
        DEPENDS ${GGXXACPR_DIR}/GGXXACPR_Win.exe libgg_patch_dev
        COMMENT "Creating ${ggxxacpr_dev}"
    )

    add_custom_target(create_gg_exe ALL DEPENDS ${ggxxacpr_dev})
    add_dependencies(create_gg_exe libgg steam_api)

    add_executable(abcbcb IMPORTED GLOBAL)
    set_target_properties(abcbcb PROPERTIES IMPORTED_LOCATION ${GGXXACPR_DIR}/GGXXACPR_Win.exe)

    add_subdirectory(test)
else()
    message(STATUS
        "Directory specified by GGXXACPR_DIR cache variable does not contain GGXXACPR_Win.exe. "
        "Debuggable GGXXACPR executable will not be created, most autotests will not be available."
    )
endif()
