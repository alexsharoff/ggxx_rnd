include(${CMAKE_CURRENT_LIST_DIR}/add_gg_test.cmake)

add_gg_test(
    REPLAY bugrepro/fafnir_tyrant_rave.ggr
    ARGS /gamemode vs2p /synctest 8
)
add_gg_test(
    REPLAY bugrepro/kliff_throw.ggr
    ARGS /gamemode vs2p /synctest 1
)
add_gg_test(
    NAME bugrepro/clash.ggr.1f
    REPLAY bugrepro/clash.ggr
    ARGS /gamemode vs2p /synctest 1
)
add_gg_test(
    NAME bugrepro/clash.ggr.8f
    REPLAY bugrepro/clash.ggr
    ARGS /gamemode vs2p /synctest 8
)
add_gg_test(
    REPLAY bugrepro/faust_taunt.ggr
    ARGS /gamemode vs2p /synctest 8
)
add_gg_test(
    REPLAY bugrepro/kliff_taunt.ggr
    ARGS /gamemode training /synctest 8
)
add_gg_test(
    REPLAY bugrepro/justice_double_nb.ggr
    ARGS /gamemode training /synctest 1
)

foreach(synctest_frames 1 2 3 4 5 6 7 8)
    set(args
        /gamemode vs2p
        /synctest ${synctest_frames}
    )

    add_gg_test(
        NAME charselect_screen.ggr.${synctest_frames}f
        REPLAY bugrepro/charselect_screen.ggr
        ARGS ${args}
    )
    add_gg_test(
        NAME charselect_screen_random_char.ggr.${synctest_frames}f
        REPLAY bugrepro/charselect_screen_random_char.ggr
        ARGS ${args}
    )
    add_gg_test(
        NAME charselect_screen_random_stage.ggr.${synctest_frames}f
        REPLAY bugrepro/charselect_screen_random_stage.ggr
        ARGS ${args}
    )

    add_gg_test(
        NAME sol_vs_ky.ggr.${synctest_frames}f
        REPLAY matches/sol_vs_ky.ggr
        ARGS ${args}
        TIMEOUT 180
    )

    add_gg_test(
        NAME faust_vs_faust.ggr.${synctest_frames}f
        REPLAY matches/faust_vs_faust.ggr
        ARGS ${args}
        TIMEOUT 120
    )

    add_gg_test(
        NAME slayer_vs_may.ggr.${synctest_frames}f
        REPLAY matches/slayer_vs_may.ggr
        ARGS ${args}
        TIMEOUT 200
    )

    add_gg_test(
        NAME session1.ggr.${synctest_frames}f
        REPLAY matches/session1.ggr
        ARGS ${args}
        TIMEOUT 400
    )

    add_gg_test(
        NAME session2.ggr.${synctest_frames}f
        REPLAY matches/session2.ggr
        ARGS ${args}
        TIMEOUT 300
    )

    add_gg_test(
        NAME session3.ggr.${synctest_frames}f
        REPLAY matches/session3.ggr
        ARGS ${args}
        TIMEOUT 400
    )

    add_gg_test(
        NAME session4.ggr.${synctest_frames}f
        REPLAY matches/session4.ggr
        ARGS ${args}
        TIMEOUT 400
    )

    add_gg_test(
        NAME session5.ggr.${synctest_frames}f
        REPLAY matches/session5.ggr
        ARGS ${args}
        TIMEOUT 600
    )

    add_gg_test(
        NAME session6.ggr.${synctest_frames}f
        REPLAY matches/session6.ggr
        ARGS ${args}
        TIMEOUT 400
    )

    add_gg_test(
        NAME session7.ggr.${synctest_frames}f
        REPLAY matches/session7.ggr
        ARGS ${args}
        TIMEOUT 400
    )

    add_gg_test(
        NAME session8.ggr.${synctest_frames}f
        REPLAY matches/session8.ggr
        ARGS ${args}
        TIMEOUT 400
    )

    add_gg_test(
        NAME session9.ggr.${synctest_frames}f
        REPLAY matches/session9.ggr
        ARGS ${args}
        TIMEOUT 600
    )

    add_gg_test(
        NAME session10.ggr.${synctest_frames}f
        REPLAY matches/session10.ggr
        ARGS ${args}
        TIMEOUT 400
    )
    add_gg_test(
        NAME 1590687408.ggr.${synctest_frames}f
        REPLAY matches/1590687408.ggr
        ARGS ${args}
        TIMEOUT 600
    )
    add_gg_test(
        NAME 1590687766.ggr.${synctest_frames}f
        REPLAY matches/1590687766.ggr
        ARGS ${args}
        TIMEOUT 600
    )
    add_gg_test(
        NAME 1590688106.ggr.${synctest_frames}f
        REPLAY matches/1590688106.ggr
        ARGS ${args}
        TIMEOUT 600
    )
    add_gg_test(
        NAME 1590688461.ggr.${synctest_frames}f
        REPLAY matches/1590688461.ggr
        ARGS ${args}
        TIMEOUT 1200
    )
    add_gg_test(
        NAME 1590690360.ggr.${synctest_frames}f
        REPLAY matches/1590690360.ggr
        ARGS ${args}
        TIMEOUT 600
    )
    add_gg_test(
        NAME 159085895311172.ggr.${synctest_frames}f
        REPLAY matches/159085895311172.ggr
        ARGS ${args}
        TIMEOUT 1200
    )
    add_gg_test(
        NAME 159085821113744.ggr.${synctest_frames}f
        REPLAY matches/159085821113744.ggr
        ARGS ${args}
        TIMEOUT 900
    )
    add_gg_test(
        NAME 15908567506164.ggr.${synctest_frames}f
        REPLAY matches/15908567506164.ggr
        ARGS ${args}
        TIMEOUT 1200
    )
    add_gg_test(
        NAME 159085566112348.ggr.${synctest_frames}f
        REPLAY matches/159085566112348.ggr
        ARGS ${args}
        TIMEOUT 600
    )
    add_gg_test(
        NAME 159085512514312.ggr.${synctest_frames}f
        REPLAY matches/159085512514312.ggr
        ARGS ${args}
        TIMEOUT 600
    )
    add_gg_test(
        NAME 159146397416240.ggr.${synctest_frames}f
        REPLAY matches/159146397416240.ggr
        ARGS ${args}
        TIMEOUT 400
    )
endforeach()
