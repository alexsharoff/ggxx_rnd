#include "game_state.h"


using mini_reflection::reflect;
using mini_reflection::member_tuple;

using memory_dump::load;
using memory_dump::dump;
using memory_dump::dump_unprotected;


template<>
struct reflect<gg_object>
{
    constexpr static auto members = member_tuple(
        &gg_object::id,
        &gg_object::facing,
        &gg_object::side,
        &gg_object::prev,
        &gg_object::next,
        &gg_object::status_bitmask,
        &gg_object::unknown_bitmask,
        &gg_object::active_move_prev,
        &gg_object::pad1,
        &gg_object::active_move,
        &gg_object::active_move_followup,
        &gg_object::active_move_frame,
        &gg_object::health,
        &gg_object::other_,
        &gg_object::last_jump_dir,
        &gg_object::unknown3,
        &gg_object::owner_id,
        &gg_object::unknown4,
        &gg_object::guard_status,
        &gg_object::char_state_ptr,
        &gg_object::unknown_callback,
        &gg_object::unknown5,
        &gg_object::sprite_array_a,
        &gg_object::sprite_array_b_idx,
        &gg_object::sprite_array_a_idx,
        &gg_object::sprite_array_b,
        &gg_object::unknown6,
        &gg_object::hitbox_array,
        &gg_object::unknown7,
        &gg_object::other1,
        &gg_object::unknown8,
        &gg_object::other2,
        &gg_object::unknown9,
        &gg_object::hitbox_count,
        &gg_object::unknown99,
        &gg_object::data_5c1_ptr,
        &gg_object::data_5c2_ptr,
        &gg_object::hit_block_callback,
        &gg_object::reset_palette_callback,
        &gg_object::unknown10,
        &gg_object::palette_status_bitmask,
        &gg_object::pos_x,
        &gg_object::pos_y,
        &gg_object::velocity_x,
        &gg_object::velocity_y,
        &gg_object::unknown11,
        &gg_object::unknown_ptr1,
        &gg_object::unknown12,
        &gg_object::unknown_ptr2,
        &gg_object::unknown_ptr3,
        &gg_object::unknown13,
        &gg_object::unknown14,
        &gg_object::unknown15,
        &gg_object::unknown16,
        &gg_object::hitstop_countdown,
        &gg_object::unknown17,
        &gg_object::data_5c1,
        &gg_object::data_5c2
    );
};

template<>
struct reflect<_match_state_1>
{
    constexpr static auto members = member_tuple(
        &_match_state_1::character_state,
        &_match_state_1::camera_state,
        &_match_state_1::player_button_timers,
        &_match_state_1::player_direction_timers,
        &_match_state_1::player_controller_state,
        &_match_state_1::data,
        &_match_state_1::char_mode_gold,
        &_match_state_1::char_mode_ex,
        &_match_state_1::char_mode_sp,
        &_match_state_1::controller_state,
        &_match_state_1::controller_state2,
        &_match_state_1::noninteractives,
        &_match_state_1::noninteractives_meta,
        &_match_state_1::p1_character,
        &_match_state_1::p2_character,
        &_match_state_1::p1_character_ptr,
        &_match_state_1::p2_character_ptr,
        &_match_state_1::projectiles,
        &_match_state_1::projectiles_ptr,
        &_match_state_1::training_mode_history,
        &_match_state_1::training_mode_cfg_display,
        &_match_state_1::training_mode_data,
        &_match_state_1::pause_state,
        &_match_state_1::extra_rng_state1,
        &_match_state_1::extra_rng_state2,

        &_match_state_1::graphics1,
        &_match_state_1::graphics2,
        &_match_state_1::graphics3,
        &_match_state_1::graphics4,
        &_match_state_1::graphics5,
        &_match_state_1::graphics6,
        &_match_state_1::graphics7,
        &_match_state_1::graphics8,
        &_match_state_1::graphics9,
        &_match_state_1::graphics10,
        &_match_state_1::graphics11,
        &_match_state_1::graphics12,
        &_match_state_1::graphics13,
        &_match_state_1::graphics14,
        &_match_state_1::graphics15,
        &_match_state_1::graphics16,
        &_match_state_1::graphics17,
        &_match_state_1::graphics18,
        &_match_state_1::graphics19,
        &_match_state_1::graphics20
    );
};

template<>
struct reflect<_match_state_2>
{
    constexpr static auto members = member_tuple(
        &_match_state_2::frame,
        &_match_state_2::p1_rounds_won,
        &_match_state_2::p2_rounds_won,
        &_match_state_2::round_end_bitmask,
        &_match_state_2::round_end_hitstop,
        &_match_state_2::match_countdown,
        &_match_state_2::round_end_flag1,
        &_match_state_2::round_end_flag2,
        &_match_state_2::round_end_flag3,
        &_match_state_2::round_end_flag4,
        &_match_state_2::round_state,
        &_match_state_2::menu_fibers,
        &_match_state_2::rng1,
        &_match_state_2::rng2,
        &_match_state_2::extra_config,
        &_match_state_2::game_mode,
        &_match_state_2::config,
        &_match_state_2::next_fiber_id,
        &_match_state_2::charselect_p1_enabled,
        &_match_state_2::charselect_p2_enabled,
        &_match_state_2::main_menu_idx,
        &_match_state_2::effect_data1,
        &_match_state_2::effect_data2,
        &_match_state_2::effect_data3,
        &_match_state_2::effect_data4,
        &_match_state_2::effect_data5,
        &_match_state_2::effect_data6,
        &_match_state_2::effect_data7,
        &_match_state_2::tiddata_fls_index,
        &_match_state_2::black_screen_opacity,
        &_match_state_2::selected_bgm,
        &_match_state_2::selected_stage,
        &_match_state_2::slayer_haiku,
        &_match_state_2::dustcombo_rng_related1,
        &_match_state_2::dustcombo_rng_related2,
        &_match_state_2::overdrive_or_round_end_rng_related1,
        &_match_state_2::overdrive_or_round_end_rng_related2,
        &_match_state_2::overdrive_or_round_end_rng_related3,
        &_match_state_2::noninteractives_rng_related1,
        &_match_state_2::round_end_rng_related1,
        &_match_state_2::noninteractives_related
    );
};

template<>
struct reflect<_match_state_3>
{
    constexpr static auto members = member_tuple(
        &_match_state_3::unknown1,
        &_match_state_3::unknown2,
        &_match_state_3::unknown3,
        &_match_state_3::unknown4,
        &_match_state_3::unknown5,
        &_match_state_3::unknown6,
        &_match_state_3::unknown8,
        &_match_state_3::unknown9,
        &_match_state_3::unknown10,
        &_match_state_3::unknown11,
        &_match_state_3::unknown12,
        &_match_state_3::unknown13,
        &_match_state_3::unknown14,
        &_match_state_3::unknown15,
        &_match_state_3::unknown16,
        &_match_state_3::unknown17,
        &_match_state_3::unknown22,
        &_match_state_3::unknown23,
        &_match_state_3::unknown24,
        &_match_state_3::unknown25,
        &_match_state_3::unknown26,
        &_match_state_3::unknown27,
        &_match_state_3::unknown28,
        &_match_state_3::unknown29,
        &_match_state_3::unknown30,
        &_match_state_3::unknown31
    );
};

template<>
struct reflect<match_state>
{
    constexpr static auto members = member_tuple(
        reflect<_match_state_1>::members,
        reflect<_match_state_2>::members,
        reflect<_match_state_3>::members
    );
};

template<>
struct reflect<gg_globals>
{
    constexpr static auto members = member_tuple(
        &gg_globals::gg_main_loop_func,
        &gg_globals::advance_frame_end_asm,
        &gg_globals::get_input_func,
        &gg_globals::draw1_func,
        &gg_globals::draw2_func,
        &gg_globals::call_fps_sleep_func_asm,
        &gg_globals::run_steam_callbacks_func_ptr,
        &gg_globals::process_audio_func,
        &gg_globals::play_sound_func,
        &gg_globals::get_current_fps_func,
        &gg_globals::direct3d9,
        &gg_globals::hwnd
    );
};

template<>
struct reflect<fiber_state>
{
    constexpr static auto members = member_tuple(
        &fiber_state::pause_state,
        &fiber_state::data1,
        &fiber_state::data2,
        &fiber_state::data3,
        &fiber_state::charselect1,
        &fiber_state::charselect2,
        &fiber_state::charselect3,
        &fiber_state::charselect4,
        &fiber_state::charselect5,
        &fiber_state::charselect6,
        &fiber_state::charselect7,
        &fiber_state::charselect8,
        &fiber_state::charselect9,
        &fiber_state::charselect10,
        &fiber_state::charselect11,
        &fiber_state::stage_select_controller,
        &fiber_state::random_stage_sequence,
        &fiber_state::random_char_sequence,
        &fiber_state::fout_condition,
        &fiber_state::data4,
        &fiber_state::data5,
        &fiber_state::data6
    );
};

void load_global_data(size_t image_base, gg_globals& state)
{
    load(image_base, state);
    state.write_cockpit_font = reinterpret_cast<write_cockpit_font_func_t*>(image_base + 0x10ECF0);
    state.write_special_font = reinterpret_cast<write_special_font_func_t*>(image_base + 0x4CFF0);
    // :base+22B280 = copy of write_utf8_font?
    state.write_utf8_font = reinterpret_cast<write_utf8_font_func_t*>(image_base + 0x22BBD0);
    state.draw_rect = reinterpret_cast<draw_rect_func_t*>(image_base + 0x25BF40);
    state.draw_pressed_buttons = reinterpret_cast<draw_pressed_buttons_func_t*>(image_base + 0x4CE00);
    state.button_bitmask_to_icon_bitmask = reinterpret_cast<button_bitmask_to_icon_bitmask_func_t*>(image_base + 0x1D42C0);
    state.direction_bitmask_to_icon_id = reinterpret_cast<direction_bitmask_to_icon_id_func_t*>(image_base + 0x1D4370);
    state.draw_arrow = reinterpret_cast<draw_arrow_func_t*>(image_base + 0x4CBE0);
    state.player_status_ticker = reinterpret_cast<player_status_ticker_func_t*>(image_base + 0x10E190);
    state.wait_file_readers = reinterpret_cast<wait_file_readers_func_t*>(image_base + 0x4B980);
    state.restart_process_func = reinterpret_cast<restart_process_func_t*>(image_base + 0x1464F0);
    state.xaudio_read_pending_files = reinterpret_cast<xaudio_read_pending_files_func_t*>(image_base + 0x233710);
    state.reset_directx_device = reinterpret_cast<reset_directx_device_func_t*>(image_base + 0x149C10);
}

void dump_global_data(size_t memory_base, const gg_globals& state)
{
    dump(state, memory_base);
}

// set palette reset bit
// current palette may be incorrect after rollback
// (lightning / fire effect)
// call this function after any kind of time travel
void set_pallette_reset_bit(game_state& state)
{
    auto& p1_char = *state.match.p1_character.get().ptr;
    if (!p1_char.palette_status_bitmask)
        p1_char.palette_status_bitmask |= 0x400;
    auto& p2_char = *state.match.p2_character.get().ptr;
    if (!p2_char.palette_status_bitmask)
        p2_char.palette_status_bitmask |= 0x400;
}

void revert_state(size_t image_base, game_state& state, fiber_mgmt::fiber_service* service)
{
    set_pallette_reset_bit(state);
    dump_unprotected(state.match, image_base);
    auto tiddata = static_cast<_tiddata*>(
        ::FlsGetValue(state.match.tiddata_fls_index.get())
    );
    tiddata->_holdrand = state.match.rand_seed;
    if (service)
    {
        for (const auto& fiber_state : state.fibers)
            service->restore(fiber_state);
    }
    dump_unprotected(state.fiber_state, image_base);

    const auto region1 = state.regions.region1.get();
    if (region1)
    {
        auto dest = *reinterpret_cast<uint64_t**>(image_base + 0x520C1C);
        for (const auto r : *region1)
        {
            *dest = r;
            ++dest;
        }
    }
}

void save_current_state(size_t image_base, game_state& state, fiber_mgmt::fiber_service* service)
{
    load(image_base, state.match);
    auto tiddata = static_cast<const _tiddata*>(
        ::FlsGetValue(state.match.tiddata_fls_index.get())
    );
    state.match.rand_seed = tiddata->_holdrand;
    state.fibers.clear();
    if (service)
    {
        for (const auto& f : state.match.menu_fibers.get())
        {
            if (f.fiber)
            {
                fiber_mgmt::fiber_state fiber_state;
                service->load(f.fiber, fiber_state);
                state.fibers.push_back(fiber_state);
            }
        }
    }
    load(image_base, state.fiber_state);

    bool reuse = false;
    if (state.regions.region1)
    {
        // Copy-on-write
        const auto& region1 = *state.regions.region1.get();
        const auto src = *reinterpret_cast<uint64_t**>(image_base + 0x520C1C);
        reuse = true;
        for (size_t i = 0; i < 0x64; ++i)
        {
            if (src[i] != region1[i])
            {
                reuse = false;
                break;
            }
        }
    }

    if (!reuse)
    {
        state.regions.region1 = std::make_shared<decltype(state.regions.region1)::element_type>();
        auto& region1 = *state.regions.region1.get();
        auto src = *reinterpret_cast<uint64_t**>(image_base + 0x520C1C);
        for (auto& r : region1)
        {
            r = *src;
            ++src;
        }
    }
}
