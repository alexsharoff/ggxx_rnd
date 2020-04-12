#include "game_state.h"

using mini_reflection::reflect;
using mini_reflection::member_tuple;

using memory_dump::load;
using memory_dump::dump;
using memory_dump::dump_unprotected;


template<>
struct reflect<active_object_state>
{
    constexpr static auto members = member_tuple(
        &active_object_state::id,
        &active_object_state::facing,
        &active_object_state::side,
        &active_object_state::prev,
        &active_object_state::next,
        &active_object_state::status_bitmask,
        &active_object_state::unknown_bitmask,
        &active_object_state::active_move_prev,
        &active_object_state::pad1,
        &active_object_state::active_move,
        &active_object_state::active_move_followup,
        &active_object_state::active_move_frame,
        &active_object_state::health,
        &active_object_state::other_,
        &active_object_state::last_jump_dir,
        &active_object_state::unknown3,
        &active_object_state::owner_id,
        &active_object_state::unknown4,
        &active_object_state::guard_status,
        &active_object_state::char_state_ptr,
        &active_object_state::unknown_callback,
        &active_object_state::unknown5,
        &active_object_state::sprite_array_a,
        &active_object_state::sprite_array_b_idx,
        &active_object_state::sprite_array_a_idx,
        &active_object_state::sprite_array_b,
        &active_object_state::unknown6,
        &active_object_state::hitbox_array,
        &active_object_state::unknown7,
        &active_object_state::other1,
        &active_object_state::unknown8,
        &active_object_state::other2,
        &active_object_state::unknown9,
        &active_object_state::hitbox_count,
        &active_object_state::unknown99,
        &active_object_state::data_5c1,
        &active_object_state::data_5c2,
        &active_object_state::hit_block_callback,
        &active_object_state::reset_palette_callback,
        &active_object_state::unknown10,
        &active_object_state::palette_status_bitmask,
        &active_object_state::pos_x,
        &active_object_state::pos_y,
        &active_object_state::velocity_x,
        &active_object_state::velocity_y,
        &active_object_state::unknown11,
        &active_object_state::unknown_ptr1,
        &active_object_state::unknown12,
        &active_object_state::unknown_ptr2,
        &active_object_state::unknown_ptr3,
        &active_object_state::unknown13,
        &active_object_state::unknown14,
        &active_object_state::unknown15,
        &active_object_state::unknown16,
        &active_object_state::hitstop_countdown,
        &active_object_state::unknown17
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
        &match_state::p1_selected_palette,
        &match_state::p2_selected_palette,
        &match_state::p1_ex_enabled,
        &match_state::p2_ex_enabled,
        &match_state::controller_state,
        &match_state::extra_objects_meta,
        &match_state::extra_objects,
        &match_state::p1_character,
        &match_state::p2_character,
        &match_state::projectiles,
        &match_state::training_mode_history,
        &match_state::training_mode_cfg_display,
        &match_state::training_mode_data,
        &match_state::pause_state,
        &match_state::extra_rng_state,

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
        /*&match_state::unknown18,
        &match_state::unknown19,
        &match_state::unknown20,
        &match_state::unknown21,*/
        &match_state::unknown22,
        &match_state::unknown23
    );
};

template<>
struct reflect<match_state_2>
{
    constexpr static auto members = member_tuple(
        &match_state_2::clock,
        &match_state_2::p1_rounds_won,
        &match_state_2::p2_rounds_won,
        &match_state_2::round_end_bitmask,
        &match_state_2::match_countdown,
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
        &match_state_2::effect_data7
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
}

void dump_global_data(size_t memory_base, const gg_state& state)
{
    dump(state, memory_base);
}

void init_fiber_mgmt()
{
    fiber_mgmt::init();
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

void revert_state(size_t image_base, game_state& state)
{
    // TODO: delete orphaned fibers

    set_pallette_reset_bit(state);
    dump_unprotected(state.match, image_base);
    dump_unprotected(state.match2, image_base);
    for (const auto& fiber_state : state.fibers)
        fiber_mgmt::dump_state(fiber_state);
}

void save_current_state(size_t image_base, game_state& state)
{
    load(image_base, state.match);
    load(image_base, state.match2);
    state.fibers.clear();
    for (const auto& f : state.match2.menu_fibers.get())
    {
        if (f.fiber)
        {
            fiber_mgmt::fiber_state fiber_state;
            fiber_mgmt::load_state(f.fiber, fiber_state);
            state.fibers.push_back(fiber_state);
        }
    }
}

uint32_t object_checksum(const active_object_state& obj, uint32_t seed = 0x83215609)
{
    return seed ^ std::hash<uint16_t>{}(obj.id)
        ^ std::hash<uint8_t>{}(obj.facing)
        ^ std::hash<uint8_t>{}(obj.side)
        ^ std::hash<uint32_t>{}(obj.status_bitmask)
        ^ std::hash<uint16_t>{}(obj.health)
        ^ std::hash<uint8_t>{}(obj.hitbox_count)
        ^ std::hash<uint32_t>{}(obj.pos_x)
        ^ std::hash<uint32_t>{}(obj.pos_y)
        ^ std::hash<uint32_t>{}(obj.velocity_x)
        ^ std::hash<uint32_t>{}(obj.velocity_y);
}

// TODO: implement proper (mini_reflection::for_each_member)
uint32_t state_checksum(const game_state& state)
{
    uint32_t hash = 0x83215609; // initial hash value, prime number
    auto& p1_char_state = state.match.character_state.get()[0];
    auto& p2_char_state = state.match.character_state.get()[1];
    const auto& p1 = state.match.p1_character.get().ptr;
    if (p1.has_value())
    {
        hash = object_checksum(p1.value(), hash)
            ^ std::hash<uint16_t>{}(p1_char_state.stun_accumulator)
            ^ std::hash<uint16_t>{}(p1_char_state.faint_countdown)
            ^ std::hash<uint16_t>{}(p1_char_state.tension)
            ^ std::hash<uint16_t>{}(p1_char_state.guard)
            ^ std::hash<uint16_t>{}(p1_char_state.burst);
    }
    const auto& p2 = state.match.p2_character.get().ptr;
    if (p2.has_value())
    {
        hash = object_checksum(p2.value(), hash)
            ^ std::hash<uint16_t>{}(p2_char_state.stun_accumulator)
            ^ std::hash<uint16_t>{}(p2_char_state.faint_countdown)
            ^ std::hash<uint16_t>{}(p2_char_state.tension)
            ^ std::hash<uint16_t>{}(p2_char_state.guard)
            ^ std::hash<uint16_t>{}(p2_char_state.burst);
    }

    uint32_t next_fiber_id = static_cast<uint32_t>(state.match2.next_fiber_id.get());
    hash ^= std::hash<uint32_t>{}(next_fiber_id);
    for (const auto& f : state.match2.menu_fibers.get())
    {
        hash ^= std::hash<uint32_t>{}(f.status);
        if (f.status)
        {
            for (char c : f.name)
            {
                hash ^= std::hash<char>{}(c);
            }
        }
    }

    const auto& rng1 = state.match2.rng1.get();
    const auto& rng2 = state.match2.rng2.get();
    // TODO: hash projectiles, extra_objs
    //const auto projectile_begin = state.match.projectiles.get().ptr.value().data;
    //const auto extra_obj_begin = state.match.extra_objects.get().ptr.value().data;
    return
        hash ^
        std::hash<uint32_t>{}(state.match2.clock.get()) ^
        std::hash<uint8_t>{}(state.match2.p1_rounds_won.get()) ^
        std::hash<uint8_t>{}(state.match2.p2_rounds_won.get()) ^
        std::hash<uint32_t>{}(state.match2.round_end_bitmask.get()) ^
        std::hash<uint16_t>{}(state.match2.match_countdown.get()) ^
        std::hash<uint8_t>{}(state.match2.round_state.get()) ^
        std::hash<uint32_t>{}(state.match2.round_end_bitmask.get()) ^
        std::hash<uint64_t>{}(rng1.index) ^
        std::hash<uint64_t>{}(rng1.data[rng1.index])^
        std::hash<uint64_t>{}(rng2.index) ^
        std::hash<uint64_t>{}(rng2.data[rng2.index]);
}
