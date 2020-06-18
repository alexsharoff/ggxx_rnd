#include "game_state_debug.h"

#include "hash.h"

#include <iomanip>
#include <iostream>
#include <string>


void object_checksum(Fnv1aHash<>& hash, const gg_object& obj, bool strict)
{
    if (strict)
    {
        gg_object obj_fixed = obj;
        // erase palette reset bit, which is manually set during rollback
        obj_fixed.palette_status_bitmask &= ~0x400u;
        hash.add(reinterpret_cast<const uint8_t*>(&obj_fixed), 0x130);
    }
    else
    {
        hash.add(obj.id).add(obj.owner_id).add(obj.pos_x).add(obj.pos_y);
        hash.add(obj.side).add(obj.status_bitmask).add(obj.velocity_x);
        hash.add(obj.velocity_y).add(obj.hitbox_count).add(obj.hitstop_countdown);
    }
}

// TODO: implement proper (mini_reflection::for_each_member)
size_t state_checksum(const game_state& state, bool strict)
{
    Fnv1aHash hash;
    auto& p1_char_state = state.match.character_state.get()[0];
    auto& p2_char_state = state.match.character_state.get()[1];
    const auto& p1 = state.match.p1_character.get().ptr;
    if (p1.has_value())
    {
        object_checksum(hash, p1.value(), strict);
        hash.add(p1_char_state);
    }
    const auto& p2 = state.match.p2_character.get().ptr;
    if (p2.has_value())
    {
        object_checksum(hash, p2.value(), strict);
        hash.add(p2_char_state);
    }

    for (size_t i = 0; i < 2; ++i)
    {
        hash.add(state.match.char_mode_ex.get()[i]);
        hash.add(state.match.char_mode_sp.get()[i]);
        hash.add(state.match.char_mode_gold.get()[i]);
    }

    const auto& projectiles_ptr = state.match.projectiles.get().ptr;
    if (projectiles_ptr.has_value())
    {
        size_t idx = 0;
        for (const auto& object : *projectiles_ptr)
        {
            if (object.id)
            {
                object_checksum(hash, object, strict);
                hash.add(idx);
            }
            ++idx;
        }
    }

    if (strict)
    {
        const auto& noninteractives_ptr = state.match.noninteractives.get().ptr;
        if (noninteractives_ptr.has_value())
        {
            size_t idx = 0;
            for (const auto& object : *noninteractives_ptr)
            {
                if (object.id)
                {
                    object_checksum(hash, object, true);
                    hash.add(idx);
                }
                ++idx;
            }
        }

        uint32_t next_fiber_id = static_cast<uint32_t>(state.match.next_fiber_id.get());
        hash.add(next_fiber_id);
        for (const auto& f : state.match.menu_fibers.get())
        {
            hash.add(f.status);
            if (f.status)
                hash.add(reinterpret_cast<const uint8_t*>(f.name), sizeof(f.name));
        }

        if (!state.fibers.empty())
        {
            hash.add(state.fiber_state.stage_select_controller.get());
            hash.add(state.fiber_state.random_stage_sequence.get()[0]);
            hash.add(state.fiber_state.random_char_sequence.get()[0]);
        }
    }

    hash.add(state.match.rand_seed);
    hash.add(state.match.selected_bgm.get());
    hash.add(state.match.selected_stage.get());
    hash.add(state.match.p1_rounds_won.get());
    hash.add(state.match.p2_rounds_won.get());
    hash.add(state.match.round_end_bitmask.get());
    hash.add(state.match.match_countdown.get());
    hash.add(state.match.round_state.get());

    const auto& data = state.match.data.get();
    hash.add(data.selected_palette[0]);
    hash.add(data.selected_palette[1]);
    hash.add(data.selected_char[0]);
    hash.add(data.selected_char[1]);
    hash.add(data.winner);
    hash.add(data.winstreak);

    const auto& rng1 = state.match.rng1.get();
    hash.add(rng1.index);
    hash.add(rng1.data[rng1.index]);

    const auto& rng2 = state.match.rng2.get();
    hash.add(rng2.index);
    hash.add(rng2.data[rng2.index]);

    return hash.get();
}

void print_hex_data(const uint8_t* data, size_t len)
{
    decltype(std::cout)::fmtflags fmtflags_backup(std::cout.flags());

    const size_t bytes_per_row = 16;
    const auto max_i = static_cast<size_t>(std::ceil(double(len) / bytes_per_row));
    for (size_t i = 0; i < max_i; ++i)
    {
        std::cout << std::setfill('0') << std::setw(2) << i;
        for (size_t j = 0; j < bytes_per_row && i * bytes_per_row + j < len; ++j)
        {
            if (j % 4 == 0)
                std::cout << ' ';
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)data[i * bytes_per_row + j];
        }
        std::cout << std::endl;
    }

    std::cout.flags(fmtflags_backup);
}

template<class T>
void print_object(const T& obj, const std::string_view& name)
{
    std::cout << "  " << name << ':' << std::endl;
    print_hex_data(reinterpret_cast<const uint8_t*>(&obj), sizeof(obj));
}

template<>
void print_object(const gg_object& obj, const std::string_view& name)
{
    gg_object obj_fixed = obj;
    // erase palette reset bit, which is manually set during rollback
    obj_fixed.palette_status_bitmask &= ~0x400u;
    std::cout << "  " << name << ':' << std::endl;
    auto data = reinterpret_cast<const uint8_t*>(&obj_fixed);
    print_hex_data(data, 0x130);
}

void print_game_state(const game_state& state)
{
    std::cout << "frame=" << state.match.frame.get() << std::endl;
    std::cout << "  checksum=" << state_checksum(state, false) << std::endl;
    std::cout << "  checksum_strict=" << state_checksum(state, true) << std::endl;
    auto& p1_char_state = state.match.character_state.get()[0];
    auto& p2_char_state = state.match.character_state.get()[1];
    const auto& p1 = state.match.p1_character.get().ptr;
    if (p1.has_value())
    {
        print_object(p1.value(), "p1");
        print_object(p1_char_state, "p1_char");
    }
    const auto& p2 = state.match.p2_character.get().ptr;
    if (p2.has_value())
    {
        print_object(p2.value(), "p2");
        print_object(p2_char_state, "p2_char");
    }

    const auto& projectiles_ptr = state.match.projectiles.get().ptr;
    if (projectiles_ptr.has_value())
    {
        size_t idx = 0;
        for (const auto& object : *projectiles_ptr)
        {
            if (object.id)
                print_object(object, ("projectile[" + std::to_string(idx) + "]"));
            ++idx;
        }
    }

    const auto& noninteractives_ptr = state.match.noninteractives.get().ptr;
    if (noninteractives_ptr.has_value())
    {
        size_t idx = 0;
        for (const auto& object : *noninteractives_ptr)
        {
            if (object.id)
                print_object(object, ("noninteractive[" + std::to_string(idx) + "]"));
            ++idx;
        }
    }

    std::cout << "  next_fiber_id=" << (uint32_t)state.match.next_fiber_id.get() << std::endl;
    for (const auto& f : state.match.menu_fibers.get())
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

    const auto& rng1 = state.match.rng1.get();
    const auto& rng2 = state.match.rng2.get();
    const auto& data = state.match.data.get();
    std::cout
        << "  rand() seed=" << state.match.rand_seed << std::endl
        << "  rng1.index=" << rng1.index << std::endl
        << "  rng1.value=" << rng1.data[rng1.index] << std::endl
        << "  rng2.index=" << rng2.index << std::endl
        << "  rng2.value=" << rng2.data[rng2.index] << std::endl
        << "  selected_stage=" << state.match.selected_stage.get() << std::endl
        << "  selected_bgm=" << state.match.selected_bgm.get() << std::endl
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
        << "  p1_rounds_won=" << (int)state.match.p1_rounds_won.get() << std::endl
        << "  p2_rounds_won=" << (int)state.match.p2_rounds_won.get() << std::endl
        << "  match_countdown=" << state.match.match_countdown.get() << std::endl
        << "  round_end_bitmask=" << state.match.round_end_bitmask.get() << std::endl
        << "  round_state=" << state.match.round_state.get() << std::endl;
}
