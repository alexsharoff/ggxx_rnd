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
struct reflect<match_state>
{
    constexpr static auto members = member_tuple(
        &match_state::character_state,
        &match_state::camera_state,
        &match_state::player_button_timers,
        &match_state::player_direction_timers,
        &match_state::player_controller_state,
        &match_state::data,
        &match_state::char_mode_gold,
        &match_state::char_mode_ex,
        &match_state::char_mode_sp,
        &match_state::controller_state,
        &match_state::controller_state2,
        &match_state::noninteractives,
        &match_state::noninteractives_meta,
        &match_state::p1_character,
        &match_state::p2_character,
        &match_state::p1_character_ptr,
        &match_state::p2_character_ptr,
        &match_state::projectiles,
        &match_state::projectiles_ptr,
        &match_state::training_mode_history,
        &match_state::training_mode_cfg_display,
        &match_state::training_mode_data,
        &match_state::pause_state,
        &match_state::extra_rng_state1,
        &match_state::extra_rng_state2,

        &match_state::graphics1,
        &match_state::graphics2,
        &match_state::graphics3,
        &match_state::graphics4,
        &match_state::graphics5,
        &match_state::graphics6,
        &match_state::graphics7,
        &match_state::graphics8,
        &match_state::graphics9,
        &match_state::graphics10,
        &match_state::graphics11,
        &match_state::graphics12,
        &match_state::graphics13,
        &match_state::graphics14,
        &match_state::graphics15,
        &match_state::graphics16,
        &match_state::graphics17,
        &match_state::graphics18,
        &match_state::graphics19,
        &match_state::graphics20,

        &match_state::unknown1,
        &match_state::unknown2,
        &match_state::unknown3,
        &match_state::unknown4,
        &match_state::unknown5,
        &match_state::unknown6,
        &match_state::unknown8,
        &match_state::unknown9,
        &match_state::unknown10,
        &match_state::unknown11,
        &match_state::unknown12,
        &match_state::unknown13,
        &match_state::unknown14,
        &match_state::unknown15,
        &match_state::unknown16,
        &match_state::unknown17,
        &match_state::unknown22,
        &match_state::unknown23,
        &match_state::unknown24,
        &match_state::unknown25,
        &match_state::unknown26,
        &match_state::unknown27,
        &match_state::unknown28,
        &match_state::unknown29,
        &match_state::unknown30
    );
};

template<>
struct reflect<match_state_2>
{
    constexpr static auto members = member_tuple(
        &match_state_2::frame,
        &match_state_2::p1_rounds_won,
        &match_state_2::p2_rounds_won,
        &match_state_2::round_end_bitmask,
        &match_state_2::round_end_hitstop,
        &match_state_2::match_countdown,
        &match_state_2::round_end_flag1,
        &match_state_2::round_end_flag2,
        &match_state_2::round_end_flag3,
        &match_state_2::round_end_flag4,
        &match_state_2::round_state,
        &match_state_2::menu_fibers,
        &match_state_2::rng1,
        &match_state_2::rng2,
        &match_state_2::extra_config,
        &match_state_2::game_mode,
        &match_state_2::config,
        &match_state_2::next_fiber_id,
        &match_state_2::charselect_p1_enabled,
        &match_state_2::charselect_p2_enabled,
        &match_state_2::main_menu_idx,
        &match_state_2::effect_data1,
        &match_state_2::effect_data2,
        &match_state_2::effect_data3,
        &match_state_2::effect_data4,
        &match_state_2::effect_data5,
        &match_state_2::effect_data6,
        &match_state_2::effect_data7,
        &match_state_2::tiddata_fls_index,
        &match_state_2::black_screen_opacity,
        &match_state_2::selected_bgm,
        &match_state_2::selected_stage,
        &match_state_2::slayer_haiku,
        &match_state_2::dustcombo_rng_related1,
        &match_state_2::dustcombo_rng_related2,
        &match_state_2::overdrive_or_round_end_rng_related1,
        &match_state_2::overdrive_or_round_end_rng_related2,
        &match_state_2::overdrive_or_round_end_rng_related3,
        &match_state_2::noninteractives_rng_related1,
        &match_state_2::round_end_rng_related1,
        &match_state_2::noninteractives_related
    );
};

template<>
struct reflect<gg_state>
{
    constexpr static auto members = member_tuple(
        &gg_state::get_raw_input_data,
        &gg_state::limit_fps,
        &gg_state::game_tick,
        &gg_state::sleep_ptr,
        &gg_state::play_sound,
        &gg_state::process_objects,
        &gg_state::process_input,
        &gg_state::direct3d9,
        &gg_state::hwnd
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

void load_global_data(size_t image_base, gg_state& state)
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
}

void dump_global_data(size_t memory_base, const gg_state& state)
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
    dump_unprotected(state.match2, image_base);
    auto tiddata = static_cast<_tiddata*>(
        ::FlsGetValue(state.match2.tiddata_fls_index.get())
    );
    tiddata->_holdrand = state.match2.rand_seed;
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
    load(image_base, state.match2);
    auto tiddata = static_cast<const _tiddata*>(
        ::FlsGetValue(state.match2.tiddata_fls_index.get())
    );
    state.match2.rand_seed = tiddata->_holdrand;
    state.fibers.clear();
    if (service)
    {
        for (const auto& f : state.match2.menu_fibers.get())
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
