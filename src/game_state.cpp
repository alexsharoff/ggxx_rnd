#include "game_state.h"

#include <iostream>


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
        &active_object_state::data_5c1_ptr,
        &active_object_state::data_5c2_ptr,
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
        &active_object_state::unknown17,
        &active_object_state::data_5c1,
        &active_object_state::data_5c2
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
        &match_state::extra_objects_meta,
        &match_state::extra_objects,
        &match_state::p1_character,
        &match_state::p2_character,
        &match_state::p1_character_ptr,
        &match_state::p2_character_ptr,
        &match_state::projectiles,
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
        /*&match_state::unknown18,
        &match_state::unknown19,
        &match_state::unknown20,
        &match_state::unknown21,*/
        &match_state::unknown22,
        &match_state::unknown23,
        &match_state::unknown24,
        &match_state::unknown25,
        &match_state::unknown26
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
        &match_state_2::extra_objects_rng_related1,
        &match_state_2::round_end_rng_related1
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

// TODO: use memory_dump::for_each_member_addr here
uint32_t object_checksum(const active_object_state& obj)
{
    return std::hash<uint16_t>{}(obj.id)
        ^ std::hash<uint8_t>{}(obj.facing)
        ^ std::hash<uint8_t>{}(obj.side)
        ^ std::hash<uint32_t>{}(obj.status_bitmask)
        ^ std::hash<uint16_t>{}(obj.health)
        ^ std::hash<uint8_t>{}(obj.hitbox_count)
        ^ std::hash<uint32_t>{}(obj.pos_x)
        ^ std::hash<uint32_t>{}(obj.pos_y)
        ^ std::hash<uint32_t>{}(obj.velocity_x)
        ^ std::hash<uint32_t>{}(obj.velocity_y)
        ^ std::hash<uint32_t>{}(obj.active_move)
        ^ std::hash<uint32_t>{}(obj.active_move_frame);
}

uint32_t object_checksum(const gg_char_state& obj)
{
    return std::hash<uint16_t>{}(obj.stun_accumulator)
        ^ std::hash<uint16_t>{}(obj.faint_countdown)
        ^ std::hash<uint16_t>{}(obj.tension)
        ^ std::hash<uint16_t>{}(obj.guard)
        ^ std::hash<uint16_t>{}(obj.burst);
}

// TODO: implement proper (mini_reflection::for_each_member)
uint32_t state_checksum(const game_state& state)
{
    uint32_t hash = 0x83215609; // initial hash value, prime number
    auto& p1_char_state = state.match.character_state.get()[0];
    auto& p2_char_state = state.match.character_state.get()[1];
    const auto& p1 = state.match.p1_character.get().ptr;
    if (p1.has_value())
        hash ^= object_checksum(p1.value()) ^ object_checksum(p1_char_state);
    const auto& p2 = state.match.p2_character.get().ptr;
    if (p2.has_value())
        hash ^= object_checksum(p2.value()) ^ object_checksum(p2_char_state);

    for (size_t i = 0; i < 2; ++i)
    {
        hash ^= std::hash<uint32_t>{}(state.match.char_mode_ex.get()[i]) ^
            std::hash<uint32_t>{}(state.match.char_mode_sp.get()[i]) ^
            std::hash<uint32_t>{}(state.match.char_mode_gold.get()[i]);
    }

    const auto& data = state.match.data.get();
    hash ^= std::hash<uint32_t>{}(data.selected_palette[0]) ^
        std::hash<uint32_t>{}(data.selected_palette[1]) ^
        std::hash<uint32_t>{}(data.selected_char[0]) ^
        std::hash<uint32_t>{}(data.selected_char[1]) ^
        std::hash<uint32_t>{}(data.winner) ^
        std::hash<uint32_t>{}(data.winstreak);

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

    if (!state.fibers.empty())
    {
        hash ^= std::hash<uint32_t>{}(state.fiber_state.stage_select_controller.get()) ^
            std::hash<uint64_t>{}(state.fiber_state.random_stage_sequence.get()[0]) ^
            std::hash<uint64_t>{}(state.fiber_state.random_char_sequence.get()[0]);
    }

    const auto& rng1 = state.match2.rng1.get();
    const auto& rng2 = state.match2.rng2.get();
    // TODO: hash projectiles, extra_objs
    //const auto projectile_begin = state.match.projectiles.get().ptr.value().data;
    //const auto extra_obj_begin = state.match.extra_objects.get().ptr.value().data;
    return
        hash ^
        std::hash<uint32_t>{}(state.match2.rand_seed) ^
        std::hash<uint32_t>{}(state.match2.selected_bgm.get()) ^
        std::hash<uint32_t>{}(state.match2.selected_stage.get()) ^
        std::hash<uint8_t>{}(state.match2.p1_rounds_won.get()) ^
        std::hash<uint8_t>{}(state.match2.p2_rounds_won.get()) ^
        std::hash<uint32_t>{}(state.match2.round_end_bitmask.get()) ^
        std::hash<uint16_t>{}(state.match2.match_countdown.get()) ^
        std::hash<uint32_t>{}(state.match2.round_state.get()) ^
        std::hash<uint64_t>{}(rng1.index) ^
        std::hash<uint64_t>{}(rng1.data[rng1.index])^
        std::hash<uint64_t>{}(rng2.index) ^
        std::hash<uint64_t>{}(rng2.data[rng2.index]);
}

void print_object(const active_object_state& obj, const std::string_view& name = "object")
{
    std::cout
        << "  " << name << "::id=" << obj.id << std::endl
        << "    " << name << "::facing=" << (int)obj.facing << std::endl
        << "    " << name << "::side=" << (int)obj.side << std::endl
        << "    " << name << "::status_bitmask=" << obj.status_bitmask << std::endl
        << "    " << name << "::health=" << obj.health << std::endl
        << "    " << name << "::hitbox_count=" << (int)obj.hitbox_count << std::endl
        << "    " << name << "::pos_x=" << obj.pos_x << std::endl
        << "    " << name << "::pos_y=" << obj.pos_y << std::endl
        << "    " << name << "::velocity_x=" << obj.velocity_x << std::endl
        << "    " << name << "::velocity_y=" << obj.velocity_y << std::endl
        << "    " << name << "::active_move=" << obj.active_move << std::endl
        << "    " << name << "::active_move_frame=" << obj.active_move_frame << std::endl;
}

void print_object(const gg_char_state& obj, const std::string_view& name = "char")
{
    std::cout
        << "    " << name << "::stun_accumulator=" << obj.stun_accumulator << std::endl
        << "    " << name << "::faint_countdown=" << obj.faint_countdown << std::endl
        << "    " << name << "::tension=" << obj.tension << std::endl
        << "    " << name << "::guard=" << obj.guard << std::endl
        << "    " << name << "::burst=" << obj.burst << std::endl;
}

// TODO: implement and use pretty_print.h here
void print_game_state(const game_state& state)
{
    std::cout << "frame=" << state.match2.frame.get() << std::endl;
    auto& p1_char_state = state.match.character_state.get()[0];
    auto& p2_char_state = state.match.character_state.get()[1];
    const auto& p1 = state.match.p1_character.get().ptr;
    if (p1.has_value())
    {
        print_object(p1.value(), "p1");
        print_object(p1_char_state, "p1");
    }
    const auto& p2 = state.match.p2_character.get().ptr;
    if (p2.has_value())
    {
        print_object(p2.value(), "p2");
        print_object(p2_char_state, "p2");
    }

    std::cout << "  next_fiber_id=" << (uint32_t)state.match2.next_fiber_id.get() << std::endl;
    for (const auto& f : state.match2.menu_fibers.get())
    {
        if (f.status)
            std::cout << "  fiber=" << f.name << ':' << f.status << std::endl;
    }

    if (!state.fibers.empty())
    {
        std::cout << "  stage_select_controller=" << state.fiber_state.stage_select_controller.get() << std::endl
            << "  random_stage_sequence[0]=" << state.fiber_state.random_stage_sequence.get()[0] << std::endl
            << "  random_char_sequence[0]=" << state.fiber_state.random_char_sequence.get()[0] << std::endl;
    }

    const auto& rng1 = state.match2.rng1.get();
    const auto& rng2 = state.match2.rng2.get();
    const auto& data = state.match.data.get();
    std::cout
        << "  rand() seed=" << state.match2.rand_seed << std::endl
        << "  rng1.index=" << rng1.index << std::endl
        << "  rng1.value=" << rng1.data[rng1.index] << std::endl
        << "  rng2.index=" << rng2.index << std::endl
        << "  rng2.value=" << rng2.data[rng2.index] << std::endl
        << "  selected_stage=" << state.match2.selected_stage.get() << std::endl
        << "  selected_bgm=" << state.match2.selected_bgm.get() << std::endl
        << "  char_mode_ex[0]=" << state.match.char_mode_ex.get()[0] << std::endl
        << "  char_mode_ex[1]=" << state.match.char_mode_ex.get()[1] << std::endl
        << "  char_mode_sp[0]=" << state.match.char_mode_sp.get()[0] << std::endl
        << "  char_mode_sp[1]=" << state.match.char_mode_sp.get()[1] << std::endl
        << "  char_mode_gold[0]=" << state.match.char_mode_gold.get()[0] << std::endl
        << "  char_mode_gold[1]=" << state.match.char_mode_gold.get()[1] << std::endl
        << "  selected_palette[0]=" << (int)data.selected_palette[0] << std::endl
        << "  selected_palette[1]=" << (int)data.selected_palette[1] << std::endl
        << "  selected_char[0]=" << data.selected_char[0] << std::endl
        << "  selected_char[1]=" << data.selected_char[1] << std::endl
        << "  winner=" << data.winner << std::endl
        << "  winstreak=" << data.winstreak << std::endl
        << "  p1_rounds_won=" << (int)state.match2.p1_rounds_won.get() << std::endl
        << "  p2_rounds_won=" << (int)state.match2.p2_rounds_won.get() << std::endl
        << "  match_countdown=" << state.match2.match_countdown.get() << std::endl
        << "  round_end_bitmask=" << state.match2.round_end_bitmask.get() << std::endl
        << "  round_state=" << state.match2.round_state.get() << std::endl;
}
