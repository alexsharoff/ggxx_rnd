#include "libgg.h"

#include <Windows.h>

#include "mini_reflection.h"
//#include "binary_serializer.h"
#include "memory_dump.h"
#include "fibers.h"

#include "network.h"

#include <D3D9.h>

#include <deque>
#include <string>
#include <utility>

using mini_reflection::reflect;
using mini_reflection::member_tuple;

using memory_dump::load;
using memory_dump::dump;
using memory_dump::ptr_chain;
using memory_dump::rel_mem_ptr;
using memory_dump::memory_offset;
using memory_dump::local_memory_accessor;
using memory_dump::offset_value;


char* g_image_base = 0;

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
struct reflect<projectiles>
{
    constexpr static auto members = member_tuple(
        &projectiles::objects
    );
};

template<>
struct reflect<match_state>
{
    constexpr static auto members = member_tuple(
        &match_state::p1_rounds_won,
        &match_state::p2_rounds_won,
        &match_state::round_end_bitmask,
        &match_state::match_countdown,
        &match_state::round_state,
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
        &match_state::unknown7,
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
        &match_state::unknown22
    );
};

uint16_t reverse_bytes(uint16_t value)
{
    return value / 256 + (value & 0xff) * 256;
}

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
        &gg_state::extra_config,
        &gg_state::menu_fibers,
        &gg_state::rng,
        &gg_state::game_mode,
        &gg_state::config,
        &gg_state::next_fiber_id,
        &gg_state::skip_saving,
        &gg_state::charselect_p1_enabled,
        &gg_state::charselect_p2_enabled,
        &gg_state::main_menu_idx
    );
};

gg_state g_state, g_state_orig;

void enable_drawing(bool enable)
{
    const auto addr = (size_t)g_image_base + 0x146FFD;
    if (enable)
    {
        // 0x75 = jne
        local_memory_accessor::write((char)0x75, addr);
    }
    else
    {
        // 0xEB = jmp
        local_memory_accessor::write((char)0xEB, addr);
    }
}

void enable_round_end_condition(bool enable)
{
    const auto addr = (size_t)g_image_base + 0x664CB;
    if (enable)
    {
        // 0F8F = jg
        const uint8_t data[] = {0x0f, 0x8f};
        local_memory_accessor::write(data, addr);
    }
    else
    {
        // 90E9 = nop jmp
        const uint8_t data[] = {0x90, 0xe9};
        local_memory_accessor::write(data, addr);
    }
}

// untested, but should work
void enable_pause_menu(bool enable)
{
    const auto addr = (size_t)g_image_base + 0xEBC19;
    if (enable)
    {
        // 0F84 = je
        const uint8_t data[] = {0x0f, 0x84};
        local_memory_accessor::write(data, addr);
    }
    else
    {
        // 90E9 = nop jmp
        const uint8_t data[] = {0x90, 0xe9};
        local_memory_accessor::write(data, addr);
    }
}

size_t g_vs_2p_jmp_addr = 0;
void __declspec(naked) jmp_menu_network()
{
    __asm {
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
    }
    network::activate();
    __asm {
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        jmp g_vs_2p_jmp_addr
    }
}

void apply_patches(char* image_base)
{
    // TODO: apply *.1337 patches from /Patches subdirectory

    // REC PLAYER => RECORD ALL
    dump("RECORD ALL", image_base + 0x3191EC);
    // REC ENEMY = FRAME STOP
    dump("FRAME STOP", image_base + 0x3191F8);
    // ENEMY JUMP = FRAME NEXT
    dump("FRAME NEXT", image_base + 0x319224);
    // ENEMY WALK = FRAME PREV
    dump("FRAME PREV", image_base + 0x319218);

    load(image_base + 0x226448 + 2 * 4, g_vs_2p_jmp_addr);
    // Replace main menu jump table entry for NETWORK
    void* ptr = jmp_menu_network;
    dump(ptr, image_base + 0x226448 + 3 * 4);
}

extern "C" __declspec(dllexport) void libgg_init()
{
    static bool s_is_ready = false;
    if (!s_is_ready)
    {
        g_image_base = (char*)::GetModuleHandle(nullptr);
        apply_patches(g_image_base);
        load(g_image_base, g_state);
        g_state.write_cockpit_font = reinterpret_cast<write_cockpit_font_func_t*>(g_image_base + 0x10ECF0);
        g_state.write_pretty_font = reinterpret_cast<write_pretty_font_func_t*>(g_image_base + 0x4CFF0);
        // :base+22B280 = copy of write_utf8_font?
        g_state.write_utf8_font = reinterpret_cast<write_utf8_font_func_t*>(g_image_base + 0x22BBD0);
        g_state.draw_rect = reinterpret_cast<draw_rect_func_t*>(g_image_base + 0x25BF40);
        g_state.draw_pressed_buttons = reinterpret_cast<draw_pressed_buttons_func_t*>(g_image_base + 0x4CE00);
        g_state.remap_action_buttons = reinterpret_cast<remap_action_buttons_func_t*>(g_image_base + 0x1D42C0);
        g_state.remap_direction_buttons = reinterpret_cast<remap_direction_buttons_func_t*>(g_image_base + 0x1D4370);
        g_state.draw_arrow = reinterpret_cast<draw_arrow_func_t*>(g_image_base + 0x4CBE0);
        g_state.player_status_ticker = reinterpret_cast<decltype(player_status_ticker)*>(g_image_base + 0x10E190);
        g_state_orig = g_state;
        s_is_ready = true;
        g_state.get_raw_input_data.get().set(get_raw_input_data);
        g_state.limit_fps.get().set(limit_fps);
        g_state.game_tick.get().set(game_tick);
        g_state.process_input.get().set(process_input);
        g_state.process_objects.get().set(process_objects);
        static auto sleep_ptr = &sleep;
        g_state.sleep_ptr = &sleep_ptr;
        if (g_state.play_sound.get().ptr)
            g_state.play_sound.get().ptr = play_sound;
        dump(g_state, g_image_base);
    }
}

bool g_invalidate_fiber_data = true;
void invalidate_fiber_data()
{
    if (g_invalidate_fiber_data)
    {
        load(g_image_base, g_state.next_fiber_id);
        load(g_image_base, g_state.menu_fibers);
        g_invalidate_fiber_data = false;
    }
}

bool in_match()
{
    invalidate_fiber_data();
    const auto& fibers = g_state.menu_fibers.get();
    for (const auto& f : fibers)
    {
        if (f.name == std::string("FIN ")) // TODO: find something less hacky
            continue;
        if (f.status)
            return false;
    }
    return g_state.next_fiber_id == fiber_id::match;
}

bool in_training_mode()
{
    load(g_image_base, g_state.game_mode);
    return g_state.game_mode == 0x101 || g_state.game_mode == 0x102;
}

uint32_t get_active_players()
{
    load(g_image_base, g_state.game_mode);
    return g_state.game_mode & 0x3;
}

void queue_destroy_fibers()
{
    invalidate_fiber_data();
    auto& fibers = g_state.menu_fibers.get();
    for (auto& f : fibers)
    {
        if (f.status)
            f.status = 3;
    }
    dump(g_state.menu_fibers, g_image_base);
}

bool find_fiber_by_name(const std::string& name)
{
    invalidate_fiber_data();
    const auto& fibers = g_state.menu_fibers.get();
    for (const auto& f : fibers)
    {
        if (f.name == name)
            return true;
    }
    return false;
}

std::deque<history_t> g_history;
bool g_recording = false;
bool g_playing = false;
int8_t g_speed = 1;
bool g_manual_frame_advance = false;
uint16_t g_prev_bitmask[] = {0, 0};
size_t g_playback_idx = 0;
size_t g_record_idx = 0;
uint16_t g_speed_control_counter = 0;
std::optional<history_t> g_prev_state;
history_t g_cur_state;
bool g_out_of_memory = false;

// set palette reset bit
// current palette may be incorrect after rollback
// (lightning / fire effect)
// call this function after any kind of time travel
void set_pallette_reset_bit(history_t& state)
{
    auto& p1_char = *std::get<0>(state).p1_character.get().ptr;
    if (!p1_char.palette_status_bitmask)
        p1_char.palette_status_bitmask |= 0x400;
    auto& p2_char = *std::get<0>(state).p2_character.get().ptr;
    if (!p2_char.palette_status_bitmask)
        p2_char.palette_status_bitmask |= 0x400;
}

void revert_state(history_t& state)
{
    set_pallette_reset_bit(state);
    const auto& ms = std::get<0>(state);
    const auto& rng = std::get<1>(state);
    dump(ms, g_image_base);
    dump(rng, g_image_base);
}

void save_current_state(const input_data& input, history_t& state)
{
    load(g_image_base, std::get<0>(state));
    load(g_image_base, std::get<1>(state));
    std::get<2>(state) = input;
}

// TODO: implement proper (mini_reflection::for_each_member)
uint32_t state_checksum(const history_t& state)
{
    const auto& ms = std::get<0>(state);
    const auto& rng = std::get<1>(state).get();
    const auto& p1 = ms.p1_character.get().ptr.value();
    const auto& p2 = ms.p2_character.get().ptr.value();
    auto& p1_char_state = ms.character_state.get()[0];
    auto& p2_char_state = ms.character_state.get()[1];
    return std::hash<uint16_t>{}(p1.health) ^
        std::hash<uint16_t>{}(p2.health) ^
        std::hash<uint16_t>{}(p1_char_state.stun_accumulator) ^
        std::hash<uint16_t>{}(p2_char_state.stun_accumulator) ^
        std::hash<uint64_t>{}(rng.index) ^
        std::hash<uint64_t>{}(rng.data[rng.index]);
}

uint16_t remap_buttons(uint16_t input, 
                       const game_config::controller_config& from,
                       const game_config::controller_config& to)
{
    input = reverse_bytes(input);

    const auto directions = from.up.bit | from.down.bit |
                            from.left.bit | from.right.bit;
    uint16_t result = input & directions;

    if (input & from.pk.bit)
        input |= from.p.bit | from.k.bit;
    if (input & from.pd.bit)
        input |= from.p.bit | from.d.bit;
    if (input & from.pks.bit)
        input |= from.p.bit | from.k.bit | from.s.bit;
    if (input & from.pksh.bit)
        input |= from.p.bit | from.k.bit | from.s.bit | from.hs.bit;

    if (input & from.p.bit)
        result |= to.p.bit;
    if (input & from.k.bit)
        result |= to.k.bit;
    if (input & from.s.bit)
        result |= to.s.bit;
    if (input & from.hs.bit)
        result |= to.hs.bit;
    if (input & from.d.bit)
        result |= to.d.bit;
    if (input & from.taunt.bit)
        result |= to.taunt.bit;
    if (input & from.reset.bit)
        result |= to.reset.bit;
    if (input & from.pause.bit)
        result |= to.pause.bit;

    return reverse_bytes(result);
}

// rec player = rec / stop recording
// rec enemy = stop world
// play memory = play / stop playing
// enemy walk = reverse, twice = max speed reverse
// enemy jump = forward, twice = max speed forward
// reset = reset current input
void __cdecl get_raw_input_data(input_data* out)
{
    const auto f = *g_state_orig.get_raw_input_data.get().get();

    input_data input;
    f(&input);
    //std::swap(input.keys[0], input.keys[1]);

    if (!network::g_is_active)
        save_current_state(input, g_cur_state);

    const auto skip =  network::callbacks::raw_input_data(input);

    if (!skip && in_match() && in_training_mode())
    {
        //memory_hook::g_capture = true;

        if (g_speed <= 0)
        {
            g_cur_state = *g_prev_state;
            revert_state(g_cur_state);
            //free_orphaned_allocations(memory_hook::g_allocations);
            //memory_hook::g_allocations.clear();
        }
        //free_memory_delayed(memory_hook::g_allocations);
        //memory_hook::g_allocations.clear();

        bool speed_control_enabled = false;
        const auto& controller_configs = get_game_config().player_controller_config;
        for (size_t i = 0; i < 2; ++i)
        {
            uint16_t bitmask = reverse_bytes(input.keys[i]);
            const auto& cfg = controller_configs[i];

            const uint16_t training_mode_buttons = reverse_bytes(
                cfg.rec_player.bit | cfg.play_memory.bit | cfg.rec_enemy.bit |
                cfg.enemy_jump.bit | cfg.enemy_walk.bit);

            input.keys[i] = input.keys[i] & ~training_mode_buttons;
            input.keys[i] = input.keys[i] & ~training_mode_buttons;

            if (g_recording || g_playing || g_manual_frame_advance)
                input.keys[i] = input.keys[i] & ~reverse_bytes(cfg.pause.bit);

            if (g_manual_frame_advance)
            {
                input.keys[i] = input.keys[i] & ~reverse_bytes(cfg.reset.bit);

                if (bitmask & cfg.reset.bit)
                {
                    input.keys[i] = 0;
                }
                else
                {
                    const uint16_t non_directional_buttons = ~reverse_bytes(
                        cfg.up.bit | cfg.down.bit | cfg.left.bit | cfg.right.bit
                    );
                    const auto& prev_keys = std::get<2>(*g_prev_state).keys[i];
                    const bool record_history_rewind = g_recording && g_record_idx + 1 < g_history.size();
                    const bool prev_has_non_directional_buttons = prev_keys & non_directional_buttons;
                    const bool is_subset = (input.keys[i] & prev_keys) == input.keys[i] && (g_prev_bitmask[i] & ~training_mode_buttons) != 0;
                    if ((record_history_rewind && input.keys[i] == 0) || (input.keys[i] == 0 || is_subset) && !(g_speed && prev_has_non_directional_buttons))
                    {
                        input.keys[i] = prev_keys;
                    }
                }
            }

            if ((bitmask & cfg.rec_player.bit) && !(g_prev_bitmask[i] & cfg.rec_player.bit))
            {
                g_playing = false;
                g_recording = !g_recording;
                if (g_recording)
                    g_history.clear();
                g_record_idx = 0;
                g_out_of_memory = false;
            }

            if ((bitmask & cfg.play_memory.bit) && !(g_prev_bitmask[i] & cfg.play_memory.bit))
            {
                g_recording = false;
                g_playing = !g_playing;
                g_playback_idx = 0;
            }

            if ((bitmask & cfg.rec_enemy.bit) && !(g_prev_bitmask[i] & cfg.rec_enemy.bit))
            {
                g_manual_frame_advance = !g_manual_frame_advance;
                g_speed = g_manual_frame_advance ? 0 : 1;
                g_out_of_memory = false;
            }

            if (bitmask & cfg.enemy_jump.bit && g_manual_frame_advance)
            {
                speed_control_enabled = true;
                if (!(g_prev_bitmask[i] & cfg.enemy_jump.bit) || g_speed_control_counter > 60)
                    g_speed = 1;
                else
                    g_speed = 0;
                ++g_speed_control_counter;
            }

            if (bitmask & cfg.enemy_walk.bit && g_manual_frame_advance)
            {
                speed_control_enabled = true;
                if (!(g_prev_bitmask[i] & cfg.enemy_walk.bit) || g_speed_control_counter > 60)
                    g_speed = -1;
                else
                    g_speed = 0;
                ++g_speed_control_counter;
            }

            g_prev_bitmask[i] = bitmask;
        }

        if (!speed_control_enabled)
        {
            if (g_manual_frame_advance)
                g_speed = 0;
            g_speed_control_counter = 0;
        }

        std::get<2>(g_cur_state) = input;

        if (g_playing)
        {
            if (g_playback_idx < g_history.size())
            {
                g_cur_state = g_history[g_playback_idx];
                input = std::get<2>(g_cur_state);
                if (g_playback_idx == 0)
                {
                    revert_state(g_cur_state);
                }

                if (g_speed < 0)
                {
                    if (g_playback_idx > 0)
                        --g_playback_idx;
                }
                else if (g_speed > 0)
                {
                    ++g_playback_idx;
                }
            }
            else
            {
                g_playing = false;
            }

            if (g_playback_idx > g_history.size() && g_manual_frame_advance)
                g_playback_idx = g_history.size();
        }

        if (g_recording && g_record_idx < 5940)
        {
            try
            {
                if (g_record_idx >= g_history.size())
                    g_history.resize(g_record_idx + 1);
                g_history[g_record_idx] = g_cur_state;

                if (g_speed < 0)
                {
                    if (g_record_idx > 0)
                        --g_record_idx;
                }
                else if (g_speed > 0)
                {
                    ++g_record_idx;
                }

                if (g_record_idx < g_history.size())
                    g_cur_state = g_history[g_record_idx];
            }
            catch(const std::bad_alloc&)
            {
                g_out_of_memory = true;
                g_recording = false;
                g_record_idx = 0;
            }
        }
        else
        {
            g_recording = false;
        }

        g_prev_state = g_cur_state;
    }
    else
    {
        if (!std::get<0>(g_cur_state).pause_state.get())
        {
            g_history.clear();
            g_recording = false;
            g_playing = false;
            g_speed = 1;
            g_manual_frame_advance = false;
            g_speed_control_counter = 0;
            g_out_of_memory = false;
        }
    }

    if (!skip)
        enable_round_end_condition(!g_recording);

    *out = input;
}

bool g_enable_fps_limit = true;
int32_t limit_fps()
{
    const auto f = *g_state_orig.limit_fps.get().get();
    return g_enable_fps_limit ? f() : 0;
}

bool g_invalidate_game_config = true;
const game_config& get_game_config()
{
    if (g_invalidate_game_config)
    {
        load(g_image_base, g_state.config);
        g_invalidate_game_config = false;
    }
    return g_state.config.get();
}

bool g_jump_to_menu = true;
void game_tick()
{
    g_invalidate_fiber_data = true;
    g_invalidate_game_config = true;
    if (!g_state.play_sound.get().ptr)
    {
        load(g_image_base, g_state.play_sound);
        if (g_state.play_sound.get().ptr)
        {
            g_state_orig.play_sound = g_state.play_sound;
            g_state.play_sound.get().ptr = play_sound;
        }
        dump(g_state.play_sound, g_image_base);
    }

    if (!g_state.direct3d9.get())
    {
        load(g_image_base, g_state.direct3d9);
    }

    if (g_jump_to_menu)
    {
        load(g_image_base, g_state.next_fiber_id);
        if (g_state.next_fiber_id.get() == fiber_id::title)
        {
            g_state.main_menu_idx = main_menu_idx::training;
            g_state.next_fiber_id = fiber_id::main_menu;
            dump(g_state.next_fiber_id, g_image_base);
            dump(g_state.main_menu_idx, g_image_base);
            g_jump_to_menu = false;
        }
    }

    if(network::callbacks::game_tick_begin())
        return;

    const auto f = *g_state_orig.game_tick.get().get();
    f();

    network::callbacks::game_tick_end();
}

void __stdcall sleep(uint32_t ms)
{
    if (network::callbacks::sleep(ms))
        return;

    const auto f = **g_state_orig.sleep_ptr.get();
    f(ms);
}

typedef struct XACT_WAVE_INSTANCE_PROPERTIES
{
} XACT_WAVE_INSTANCE_PROPERTIES, *LPXACT_WAVE_INSTANCE_PROPERTIES;

DECLARE_INTERFACE(IXACT3Wave)
{
    STDMETHOD(Destroy)(THIS) PURE;
    STDMETHOD(Play)(THIS) PURE;
    STDMETHOD(Stop)(THIS_ DWORD dwFlags) PURE;
    STDMETHOD(Pause)(THIS_ BOOL fPause) PURE;
    STDMETHOD(GetState)(THIS_ __out DWORD* pdwState) PURE;
    STDMETHOD(SetPitch)(THIS_ SHORT pitch) PURE;
    STDMETHOD(SetVolume)(THIS_ FLOAT volume) PURE;
    STDMETHOD(SetMatrixCoefficients)(THIS_ UINT32 uSrcChannelCount, UINT32 uDstChannelCount, __in float* pMatrixCoefficients) PURE;
    STDMETHOD(GetProperties)(THIS_ __out LPXACT_WAVE_INSTANCE_PROPERTIES pProperties) PURE;
};

struct IXACT3WaveBank {};

// -----------------------------------------------------------------------------
// XACT State flags
// -----------------------------------------------------------------------------
static const DWORD XACT_STATE_CREATED           = 0x00000001; // Created, but nothing else
static const DWORD XACT_STATE_PREPARING         = 0x00000002; // In the middle of preparing
static const DWORD XACT_STATE_PREPARED          = 0x00000004; // Prepared, but not yet played
static const DWORD XACT_STATE_PLAYING           = 0x00000008; // Playing (though could be paused)
static const DWORD XACT_STATE_STOPPING          = 0x00000010; // Stopping
static const DWORD XACT_STATE_STOPPED           = 0x00000020; // Stopped
static const DWORD XACT_STATE_PAUSED            = 0x00000040; // Paused (Can be combined with some of the other state flags above)
static const DWORD XACT_STATE_INUSE             = 0x00000080; // Object is in use (used by wavebanks and soundbanks).
static const DWORD XACT_STATE_PREPAREFAILED     = 0x80000000; // Object preparation failed.

struct XACT3Wave : public IXACT3Wave
{
    STDMETHOD(Destroy)(THIS)
    {
        return S_OK;
    }
    STDMETHOD(Play)(THIS)
    {
        return S_OK;
    }
    STDMETHOD(Stop)(THIS_ DWORD dwFlags)
    {
        return S_OK;
    }
    STDMETHOD(Pause)(THIS_ BOOL fPause)
    {
        return S_OK;
    }
    STDMETHOD(GetState)(THIS_ __out DWORD* pdwState)
    {
        *pdwState = XACT_STATE_STOPPED;
        return S_OK;
    }
    STDMETHOD(SetPitch)(THIS_ SHORT pitch)
    {
        return S_OK;
    }
    STDMETHOD(SetVolume)(THIS_ FLOAT volume)
    {
        return S_OK;
    }
    STDMETHOD(SetMatrixCoefficients)(THIS_ UINT32 uSrcChannelCount, UINT32 uDstChannelCount, __in float* pMatrixCoefficients)
    {
        return S_OK;
    }
    STDMETHOD(GetProperties)(THIS_ __out LPXACT_WAVE_INSTANCE_PROPERTIES pProperties)
    {
        return S_OK;
    }
};

XACT3Wave w;
// IXACT3WaveBank_Play(__in IXACT3WaveBank* pWaveBank, XACTINDEX nWaveIndex, DWORD dwFlags, DWORD dwPlayOffset, XACTLOOPCOUNT nLoopCount, __deref_out IXACT3Wave** ppWave)
int32_t __stdcall play_sound(IXACT3WaveBank* a1, int16_t a2, uint32_t a3, int32_t a4, int8_t a5, IXACT3Wave** a6)
{
    if (!network::callbacks::play_sound(a1, a2))
    {
        const auto f = *g_state_orig.play_sound.get().ptr;
        return f(a1, a2, a3, a4, a5, a6);
    }
    else
    {
        *a6 = &w;
        return 0;
    }
}

void player_status_ticker(const char* message, uint32_t side)
{
    auto f = g_state.player_status_ticker;
    // Custom calling convention due to LTCG:
    // * Message in ESI
    // * Side (0, 1) in EAX
    __asm
    {
        mov esi, message
        mov eax, side
        call f
    }
}

void process_input()
{
    const auto f = *g_state_orig.process_input.get().get();
    f();
}

void write_utf8_font(const char* text, int x, int y,
                     float z, float opacity,
                     uint32_t unknown3, uint32_t unknown4,
                     float scale_x, float scale_y)
{
    float scale_x_orig, scale_y_orig;
    load(g_image_base + 0x3EE774, scale_x_orig);
    load(g_image_base + 0x3EE83C, scale_y_orig);
    dump(scale_x, g_image_base + 0x3EE774);
    dump(scale_y, g_image_base + 0x3EE83C);
    auto f = g_state.write_utf8_font;
    __asm
    {
        push unknown4
        push unknown3
        push opacity
        push z
        push y
        push x
        mov eax, text
        call f
        add esp, 4*6
    }
    dump(scale_x_orig, g_image_base + 0x3EE774);
    dump(scale_y_orig, g_image_base + 0x3EE83C);
}

void draw_rect(uint32_t color, uint32_t x1, uint32_t y1,
               uint32_t x2, uint32_t y2, uint32_t unknown)
{
    auto f = g_state.draw_rect;
    __asm
    {
        push unknown
        push y2
        push x2
        push y1
        push x1
        mov eax, color
        call f
        add esp, 4*5
    }
}

uint32_t remap_action_buttons(uint32_t input, const active_object_state* obj)
{
    auto f = g_state.remap_action_buttons;
    __asm
    {
        mov ecx, input
        mov edx, obj
        push 1
        call f
        add esp, 4
    }
}

uint32_t remap_direction_buttons(uint32_t input)
{
    uint32_t input_ = 0;
    // additional preprocessing is required
    if (input & 0x10)
        input_ |= 4;
    if (input & 0x20)
        input_ |= 2;
    if (input & 0x40)
        input_ |= 8;
    if (input & 0x80)
        input_ |= 1;

    auto f = g_state.remap_direction_buttons;
    __asm
    {
        mov ecx, input_
        push 0
        push input_
        call f
        add esp, 4*2
    }
}

void process_objects()
{
    const auto f = *g_state_orig.process_objects.get().get();
    f();
    match_state ms;
    if (g_manual_frame_advance)
    {
        load(g_image_base, ms.controller_state);
        const auto& controller_state = ms.controller_state.get();
        if (controller_state[0].bitmask_cur)
        {
            const auto input = controller_state[0].bitmask_cur;
            const auto obj = (active_object_state**)(g_image_base + 0x516778);
            const auto directions = remap_direction_buttons(input);
            g_state.draw_pressed_buttons(directions, 20, 300, 3, 1);
            const auto buttons = remap_action_buttons(input, *obj);
            g_state.draw_pressed_buttons(buttons, 50, 300, 3, 1);
        }
        if (controller_state[1].bitmask_cur)
        {
            const auto input = controller_state[1].bitmask_cur;
            const auto obj = (active_object_state**)(g_image_base + 0x51A07C);
            const auto directions = remap_direction_buttons(input);
            g_state.draw_pressed_buttons(directions, 550, 300, 3, 1);
            const auto buttons = remap_action_buttons(input, *obj);
            g_state.draw_pressed_buttons(buttons, 580, 300, 3, 1);
        }
    }
    g_state.write_cockpit_font("DEV BUILD", 50, 100, 1, 0x50, 1);
    if (g_recording)
    {
        auto str = "REC " + std::to_string(g_record_idx);
        g_state.write_cockpit_font(str.c_str(), 285, 100, 1, 0xFF, 1);
    }
    if (g_playing)
    {
        auto str = "PLAY " + std::to_string(g_playback_idx);
        g_state.write_cockpit_font(str.c_str(), 285, 100, 1, 0xFF, 1);
    }
    if (g_manual_frame_advance)
    {
        g_state.write_cockpit_font("FRAME STOP", 285, 150, 1, 0xFF, 1);
    }
    if (g_out_of_memory)
    {
        g_state.write_cockpit_font("OUT OF MEMORY!", 50, 150, 1, 0xff, 1);
    }

    const auto is_paused = std::get<0>(g_cur_state).pause_state.get();
    const auto display_damage = std::get<0>(g_cur_state).training_mode_cfg_display.get() & 2;
    if (in_match() && in_training_mode() && !is_paused && display_damage)
    {
        load(g_image_base, ms.character_state);
        auto player_idx = get_active_players();
        if (player_idx == 2)
            player_idx = 0;
        else
            player_idx = 1;
        const auto& enemy_char_state = ms.character_state.get()[player_idx];
        const auto stun_accumulator = enemy_char_state.stun_accumulator;
        const auto stun_resistance = enemy_char_state.stun_resistance;
        const auto faint_countdown = enemy_char_state.faint_countdown;

        const int increment_y = 0x18;
        const int key_x = player_idx ? 0x16a : 0x2a;
        const int value_x = player_idx ? 0x21a : 0xda;
        const float key_z = 265;
        const float value_z = 266;
        int y = 0xb8;
        {
            y += increment_y;
            g_state.write_cockpit_font("        STUN", key_x, y, key_z, 0xff, 1);
            if (stun_accumulator != 0xffff)
            {
                char str[8];
                auto begin = std::begin(str);
                const auto end = std::end(str);
                begin = std::to_chars(
                    begin, end, stun_accumulator / 100
                ).ptr;
                *begin = '/';
                begin = std::to_chars(
                    begin + 1, end, stun_resistance
                ).ptr;
                *begin = 0;
                g_state.write_cockpit_font(str, value_x, y, value_z, 0xff, 1);
            }
            else
            {
                g_state.write_cockpit_font("FAINT", value_x, y, value_z, 0xff, 1);
            }

            if (faint_countdown)
            {
                y += increment_y;
                g_state.write_cockpit_font("       FAINT", key_x, y, key_z, 0xff, 1);
                {
                    char str[6];
                    format_int(str, faint_countdown);
                    g_state.write_cockpit_font(str, value_x, y, value_z, 0xff, 1);
                }
            }
        }
    }
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    };
    return TRUE;
}
