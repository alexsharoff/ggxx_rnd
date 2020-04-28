include(add_gg_test)

# Enable once [!] after game state has been extended and all autotests started to fail
set(LIBGG_TEST_UPDATE_STATE_CHECKSUM OFF)

add_gg_test(
    REPLAY bugrepro/fafnir_tyrant_rave.ggr
    ARGS --gamemode network --checkstate --synctest 8
)
add_gg_test(
    REPLAY bugrepro/kliff_throw.ggr
    ARGS --gamemode network --checkstate --synctest 1
)
add_gg_test(
    REPLAY bugrepro/faust_taunt.ggr
    ARGS
        --checkstate
        --rollback 1400 1050
        --rollback 1460 1050
)
add_gg_test(
    REPLAY bugrepro/kliff_taunt.ggr
    ARGS
        --checkstate
        --rollback 840 691
        --rollback 1289 840
        --rollback 1300 1288
)

foreach(synctest_frames 1 2 3 4 5 6 7 8)
    set(args
        --gamemode network
        --checkstate
        --synctest ${synctest_frames}
    )
    add_gg_test(
        NAME charselect_screen_${synctest_frames}f
        REPLAY bugrepro/charselect_screen.ggr
        ARGS ${args}
    )
    add_gg_test(
        NAME charselect_screen_random_char_${synctest_frames}f
        REPLAY bugrepro/charselect_screen_random_char.ggr
        ARGS ${args}
    )
    add_gg_test(
        NAME charselect_screen_random_stage_${synctest_frames}f
        REPLAY bugrepro/charselect_screen_random_stage.ggr
        ARGS ${args}
    )
endforeach()

add_gg_test(
    REPLAY matches/sol_vs_ky.ggr
    ARGS --gamemode network --checkstate --synctest 1
    TIMEOUT 120
)
