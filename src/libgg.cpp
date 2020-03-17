#include "libgg.h"

#include <Windows.h>

#include <cstdint>

#include "mini_reflection.h"
//#include "binary_serializer.h"
#include "memory_dump.h"

#include "ggxxacpr.h"
#include "network.h"

#include <Dbghelp.h>
#include <Windows.h>
#include <objbase.h>

#include <D3D9.h>

#include <array>
#include <charconv>
#include <deque>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using mini_reflection::reflect;
using mini_reflection::member_tuple;

using memory_dump::load;
using memory_dump::dump;
using memory_dump::ptr_chain;
using memory_dump::rel_mem_ptr;
using memory_dump::memory_offset;
using memory_dump::local_memory_accessor;
using memory_dump::offset_value;


void* PatchIAT(HMODULE module, void* oldSymbol, void* newSymbol)
{
    ULONG size = 0;
    unsigned char* baseAddress = (unsigned char*)module;
    void* originalSymbol = NULL;

    IMAGE_IMPORT_DESCRIPTOR* importDescriptor = (IMAGE_IMPORT_DESCRIPTOR*)::ImageDirectoryEntryToData(
        baseAddress, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);
    if (importDescriptor == NULL)
    {
        //Patching module has no IDT.
        return NULL;
    }

    while (importDescriptor->FirstThunk != 0)
    {
        IMAGE_THUNK_DATA* thunk = (IMAGE_THUNK_DATA*)(baseAddress + importDescriptor->FirstThunk);
        while (thunk->u1.Function != 0)
        {
            if (thunk->u1.Function != (DWORD_PTR)oldSymbol) 
            {
                thunk++;
                continue;
            }

            originalSymbol = (LPVOID)thunk->u1.Function;
            DWORD protect;
            ::VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function), PAGE_EXECUTE_READWRITE, &protect);
            thunk->u1.Function = (DWORD_PTR)newSymbol;
            ::VirtualProtect(&thunk->u1.Function, sizeof(thunk->u1.Function), protect, &protect);
        }
        importDescriptor++;
    }

    return originalSymbol;
}

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
struct reflect<directx_obj>
{
    constexpr static auto members = member_tuple(
        &directx_obj::vtable1,
        &directx_obj::vtable2,
        &directx_obj::ptr1,
        &directx_obj::ptr2,
        &directx_obj::idx1,
        &directx_obj::idx2
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

void EnableDrawing(bool enable)
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

void EnableRoundEndCondition(bool enable)
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
void EnablePauseMenu(bool enable)
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

namespace memory_hook
{

HLOCAL WINAPI LocalFree(HLOCAL hMem);
HLOCAL WINAPI LocalAlloc(UINT uFlags, SIZE_T uBytes);

decltype(LocalFree)* g_local_free_impl = nullptr;
decltype(LocalAlloc)* g_local_alloc_impl = nullptr;
bool g_capture = false;

struct alloc_info
{
    HLOCAL hMem;
    bool free;
};
std::vector<alloc_info> g_allocations;

HLOCAL WINAPI LocalFree(HLOCAL hMem)
{
    if (g_capture)
    {
        g_allocations.push_back({hMem, true});
        return NULL;
    }
    else
    {
        return g_local_free_impl(hMem);
    }
}

HLOCAL WINAPI LocalAlloc(UINT uFlags, SIZE_T uBytes)
{
    HLOCAL res = g_local_alloc_impl(uFlags, uBytes);
    if (g_capture)
        g_allocations.push_back({res, false});
    return res;
}

}

size_t g_vs_2p_jmp_addr = 0;
void __declspec(naked) jmp_menu_network()
{
    network::start();
    __asm {
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

        memory_hook::g_local_free_impl = PatchIAT(
            "kernel32.dll", "LocalFree", "d3d9.dll", memory_hook::LocalFree
        );
        memory_hook::g_local_alloc_impl = PatchIAT(
            "kernel32.dll", "LocalAlloc", "d3d9.dll", memory_hook::LocalAlloc
        );
    }
}

bool g_enable_fps_limit = true;


bool in_match()
{
    const auto& fibers = g_state.menu_fibers.get();
    for (const auto& f : fibers)
    {
        if (f.name == std::string("FIN ")) // TODO: find something less hacky
            continue;
        if (f.status)
            return false;
    }
    return true;
}

void queue_destroy_fibers()
{
    auto& fibers = g_state.menu_fibers.get();
    for (auto& f : fibers)
    {
        if (f.status)
            f.status = 3;
    }
    dump(g_state.menu_fibers, g_image_base);
}

void free_orphaned_allocations(const std::vector<memory_hook::alloc_info>& items)
{
    for (const auto i: items)
    {
        if (!i.free)
            ::LocalFree(i.hMem);
    }
}

void free_memory_delayed(const std::vector<memory_hook::alloc_info>& items)
{
    for (const auto i: items)
    {
        if (i.free)
            ::LocalFree(i.hMem);
    }
}

std::deque<history_t> g_history;
bool g_recording = false;
bool g_playing = false;
// 0: frame step
// (-)1: normal (reverse)
// (-)2: disable fps limit (reverse)
// (-)3: disable drawing (reverse)
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

    load(g_image_base, g_state.menu_fibers);

    if (!in_match())
        network::stop();

    load(g_image_base, g_state.game_mode);

    load(g_image_base, std::get<0>(g_cur_state));

    auto& p1_char_optional = std::get<0>(g_cur_state).p1_character.get().ptr;
    const auto training_mode = g_state.game_mode == 0x101;
    if (in_match() && training_mode && p1_char_optional)
    {
        //memory_hook::g_capture = true;

        if (g_speed <= 0)
        {
            g_cur_state = *g_prev_state;
            set_pallette_reset_bit(g_cur_state);
            const auto& ms = std::get<0>(g_cur_state);
            const auto& rng = std::get<1>(g_cur_state);
            dump(ms, g_image_base);
            dump(rng, g_image_base);

            //free_orphaned_allocations(memory_hook::g_allocations);
            //memory_hook::g_allocations.clear();
        }
        else
        {
            load(g_image_base, std::get<1>(g_cur_state));
        }
        //free_memory_delayed(memory_hook::g_allocations);
        //memory_hook::g_allocations.clear();

        bool speed_control_enabled = false;
        const auto& controller_configs = g_state.config.get().player_controller_config;
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
                {
                    g_speed = 1;
                }
                else
                {
                    g_speed = 0;
                }
                ++g_speed_control_counter;
            }

            if (bitmask & cfg.enemy_walk.bit && g_manual_frame_advance)
            {
                speed_control_enabled = true;
                if (!(g_prev_bitmask[i] & cfg.enemy_walk.bit) || g_speed_control_counter > 60)
                {
                    g_speed = -1;
                }
                else
                {
                    g_speed = 0;
                }
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

        bool drawing_enabled = true;
        bool fps_limit_enabled = true;
        if (std::abs(g_speed) == 2)
            fps_limit_enabled = false;
        if (std::abs(g_speed) == 3)
            drawing_enabled = false;

        EnableDrawing(drawing_enabled);
        g_enable_fps_limit = fps_limit_enabled;

        std::get<2>(g_cur_state) = input;

        if (g_playing)
        {
            if (g_playback_idx < g_history.size())
            {
                g_cur_state = g_history[g_playback_idx];
                input = std::get<2>(g_cur_state);
                if (g_playback_idx == 0)
                {
                    set_pallette_reset_bit(g_cur_state);
                    const auto& ms = std::get<0>(g_cur_state);
                    const auto& rng = std::get<1>(g_cur_state);
                    dump(ms, g_image_base);
                    dump(rng, g_image_base);
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
            memory_hook::g_capture = false;
            memory_hook::g_allocations.clear();
        }
    }

    EnableRoundEndCondition(!g_recording);

    *out = input;
}

int32_t limit_fps()
{
    const auto f = *g_state_orig.limit_fps.get().get();
    return g_enable_fps_limit ? f() : 0;
}

void game_tick()
{
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

    load(g_image_base, g_state.config);

    load(g_image_base, g_state.next_fiber_id);
    if (g_state.next_fiber_id.get() == fiber_id::title)
    {
        g_state.main_menu_idx = main_menu_idx::training;
        g_state.next_fiber_id = fiber_id::main_menu;
        dump(g_state.next_fiber_id, g_image_base);
        dump(g_state.main_menu_idx, g_image_base);
    }

    const auto f = *g_state_orig.game_tick.get().get();
    f();
}

void __stdcall sleep(uint32_t ms)
{
    const auto f = **g_state_orig.sleep_ptr.get();
    f(ms);
}

XACT3Wave w;
std::unordered_map<IXACT3WaveBank*, std::unordered_set<int16_t>> known_sounds;
int32_t __stdcall play_sound(IXACT3WaveBank* a1, int16_t a2, uint32_t a3, int32_t a4, int8_t a5, IXACT3Wave** a6)
{
    auto [_, success] = known_sounds[a1].insert(a2);
    if (success)
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

    const auto training_mode = g_state.game_mode == 0x101;
    const auto is_paused = std::get<0>(g_cur_state).pause_state.get();
    const auto display_damage = std::get<0>(g_cur_state).training_mode_cfg_display.get() & 2;
    if (in_match() && training_mode && !is_paused && display_damage)
    {
        load(g_image_base, ms.character_state);
        const auto& p2_char_state = ms.character_state.get()[1];
        const auto stun_accumulator = p2_char_state.stun_accumulator;
        const auto stun_resistance = p2_char_state.stun_resistance;
        const auto faint_countdown = p2_char_state.faint_countdown;

        const int increment_y = 0x18;
        const int key_x = 0x16a;
        const int value_x = 0x21a;
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
