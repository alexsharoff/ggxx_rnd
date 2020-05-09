include(add_gg_test)

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
endforeach()
