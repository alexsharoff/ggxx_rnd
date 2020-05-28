#include "game_state_debug.h"

#include "util.h"

#include <iomanip>
#include <iostream>
#include <string>


void object_checksum(Fnv1aHash<>& hash, const active_object_state& obj)
{
    active_object_state obj_fixed = obj;
    // erase palette reset bit, which is manually set during rollback
    obj_fixed.palette_status_bitmask &= ~0x400u;
    hash.add(reinterpret_cast<const uint8_t*>(&obj_fixed), 0x130);
}

// TODO: implement proper (mini_reflection::for_each_member)
size_t state_checksum(const game_state& state)
{
    Fnv1aHash hash;
    auto& p1_char_state = state.match.character_state.get()[0];
    auto& p2_char_state = state.match.character_state.get()[1];
    const auto& p1 = state.match.p1_character.get().ptr;
    if (p1.has_value())
    {
        object_checksum(hash, p1.value());
        hash.add(p1_char_state);
    }
    const auto& p2 = state.match.p2_character.get().ptr;
    if (p2.has_value())
    {
        object_checksum(hash, p2.value());
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
                object_checksum(hash, object);
                hash.add(idx);
            }
            ++idx;
        }
    }

    const auto& noninteractives_ptr = state.match.extra_objects.get().ptr;
    if (noninteractives_ptr.has_value())
    {
        size_t idx = 0;
        for (const auto& object : *noninteractives_ptr)
        {
            if (object.id)
            {
                object_checksum(hash, object);
                hash.add(idx);
            }
            ++idx;
        }
    }

    const auto& data = state.match.data.get();
    hash.add(data.selected_palette[0]);
    hash.add(data.selected_palette[1]);
    hash.add(data.selected_char[0]);
    hash.add(data.selected_char[1]);
    hash.add(data.winner);
    hash.add(data.winstreak);

    uint32_t next_fiber_id = static_cast<uint32_t>(state.match2.next_fiber_id.get());
    hash.add(next_fiber_id);
    for (const auto& f : state.match2.menu_fibers.get())
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

    const auto& rng1 = state.match2.rng1.get();
    const auto& rng2 = state.match2.rng2.get();
    hash.add(state.match2.rand_seed);
    hash.add(state.match2.selected_bgm.get());
    hash.add(state.match2.selected_stage.get());
    hash.add(state.match2.p1_rounds_won.get());
    hash.add(state.match2.p2_rounds_won.get());
    hash.add(state.match2.round_end_bitmask.get());
    hash.add(state.match2.match_countdown.get());
    hash.add(state.match2.round_state.get());
    hash.add(rng1.index);
    hash.add(rng1.data[rng1.index]);
    hash.add(rng2.index);
    hash.add(rng2.data[rng2.index]);
    return hash.get();
}

void print_object(const active_object_state& obj, const std::string_view& name = "object")
{
    decltype(std::cout)::fmtflags fmtflags_backup(std::cout.flags());

    active_object_state obj_fixed = obj;
    // erase palette reset bit, which is manually set during rollback
    obj_fixed.palette_status_bitmask &= ~0x400u;
    std::cout << "  " << name << ':' << std::endl;
    auto data = reinterpret_cast<const uint8_t*>(&obj_fixed);
    for (size_t i = 0; i < 0x13; ++i)
    {
        std::cout << std::setfill('0') << std::setw(2) << i;
        for (size_t j = 0; j < 16; ++j)
        {
            if (j % 4 == 0)
                std::cout << ' ';
            std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)data[i * 16 + j];
        }
        std::cout << std::endl;
    }

    std::cout.flags(fmtflags_backup);
}

void print_object(const gg_char_state& obj, const std::string_view& name = "char")
{
    std::cout
        << "  " << name << ':' << std::endl
        << "    stun_accumulator=" << obj.stun_accumulator << std::endl
        << "    faint_countdown=" << obj.faint_countdown << std::endl
        << "    tension=" << obj.tension << std::endl
        << "    guard=" << obj.guard << std::endl
        << "    burst=" << obj.burst << std::endl;
}

void print_game_state(const game_state& state)
{
    std::cout << "frame=" << state.match2.frame.get() << std::endl;
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

    const auto& noninteractives_ptr = state.match.extra_objects.get().ptr;
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
