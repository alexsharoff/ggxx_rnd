#pragma once

#include <Windows.h>

#include <cstdint>

#include "mini_reflection.h"
//#include "binary_serializer.h"
#include "memory_dump.h"

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

using memory_dump::ptr_chain;
using memory_dump::rel_mem_ptr;
using memory_dump::memory_offset;
using memory_dump::offset_value;


void* PatchIAT(HMODULE module, void* oldSymbol, void* newSymbol);

template<class F>
F* PatchIAT(LPCSTR symbolModule, LPCSTR symbolName, LPCSTR iatModuleName, F* replacement)
{
    void* import = ::GetProcAddress(::GetModuleHandleA(symbolModule), symbolName);
    if (import == NULL)
        return NULL;

    return (F*)PatchIAT(::GetModuleHandleA(iatModuleName), import, replacement);
}

struct input_data
{
    uint16_t keys[2];
    struct joystick
    {
        uint8_t axes[4];
    } joysticks[4];
    uint32_t unknown[2];
};

extern char* g_image_base;

void __cdecl get_raw_input_data(input_data* out);
int32_t limit_fps();

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
// IXACT3WaveBank_Play(__in IXACT3WaveBank* pWaveBank, XACTINDEX nWaveIndex, DWORD dwFlags, DWORD dwPlayOffset, XACTLOOPCOUNT nLoopCount, __deref_out IXACT3Wave** ppWave)
int32_t __stdcall play_sound(IXACT3WaveBank*, int16_t, uint32_t, int32_t, int8_t, IXACT3Wave**);
void game_tick();
void __stdcall sleep(uint32_t ms);
typedef int (__cdecl write_cockpit_font_func_t)(const char* buffer, int x, int y, float z, uint8_t alpha, float scale);
// Example arguments:
// TRAINING MENU, 42000000, 42200000, 40000000, 1, 5, 3f800000
// PLAYER, 42800000, 42A00000, 40000000, 1, 5, 3F800000
// ENEMY, 43C00000, 42A00000, 40000000, A0, 5, 3F800000
// H-SLASH, 42000000, 43480000, 40000000,A0, 7, 3F800000
// SWITCH, 43830000, 42C00000, 40800000, 1, 6, 3F800000
// ver R , 44070000 , 43A30000, 41880000, 1, 0x11, 3F000000
typedef void (__cdecl write_pretty_font_func_t)(
    const char* text, float x, float y, float z,
    uint32_t flags, uint32_t font, float scale
);
// Example arguments (EAX = ptr to utf-8 text):
// (HELP & OPTIONS), 140, 56, 41400000, 3F800000, FFFFFFFF, 0
// (GAME SETTINGS), 140, 84, 41400000, 3F800000, FFFFFFFF, 0
// (CONTROLLER SETTINGS), 140, 9c, 41400000, 402A0000, FFFFFFFF, 0
// (FRANÇAIS), 198, 138 ,40000000, 3F800000, FFFF7A01, 0
// (Sign in have been changed), 280, 120, 0, 3F800000, FFFFFFFF, 0
// (Quit game and return to main menu?), 140, b0, 3F800000, 3F800000, FFFFFFFF, 0
// Uses global values:
// :base+3EE774 (float): font x scale
// :base+3EE83C (float): font y scale
typedef void (__cdecl write_utf8_font_func_t)(
    int x, int y, float z, float opacity, uint32_t unknown3, uint32_t unknown4
);
// Color in EAX: 0xAARRGGBB
typedef void (__cdecl draw_rect_func_t)(
    uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t unknown
);
// unknown1 = 3, unknown2 = 1
// process_objects() deletes previously drawn buttons
// input: first byte = direction bitmask, second byte = action bitmask
// input must not contain both directions and actions
// split input and call draw_pressed_buttons_func_t two times if needed
typedef void (__cdecl draw_pressed_buttons_func_t)(
    uint32_t input, uint32_t x, uint32_t y, uint32_t unknown1, uint32_t unknown2
);
// Remap action buttons according to player's controller config.
// mode: 0 (controller), 1-4 (presets). Value doesn't really matter,
// as long as it's not 0. Passing 0 results in incorrect button mapping.
// This is due to a bug in GG: set MODE to CONTROLLER and DISPLAY to INPUT,
// displayed buttons will be incorrect.
// input registers:
// * ecx: copy of input
// * edx: active_object_state* for current player
// output registers:
// * eax: result
typedef uint32_t (__cdecl remap_action_buttons_func_t)(uint32_t mode);
// This function is doing some useless bit-swapping, but it's required for
// draw_pressed_buttons_func_t.
// unknown = 0
// input registers:
// * ecx: copy of input
// output registers:
// * eax: result
typedef uint32_t (__cdecl remap_direction_buttons_func_t)(uint32_t input, uint32_t unknown);
// Draw an arrow icon. For example, DISPLAY option in training mode pause menu.
// unknown = 2
typedef void (__cdecl draw_arrow_func_t)(
    uint32_t arrow_type, uint32_t x, uint32_t y, uint32_t unknown, uint32_t alpha
);
void player_status_ticker(const char* message, uint32_t side);
void process_input();
void process_objects();

typedef int (fiber_func_t)();

struct menu_fiber
{
    uint32_t status; // 0
    uint32_t unknown[5]; // 4
    char name[0x18]; // // 18
    fiber_func_t* func;  //30
    LPVOID fiber; // 34
}; // 38

static_assert(sizeof(menu_fiber) == 0x38);

#pragma pack(push, 1)
struct gg_char_state
{
    // common state + char-specific stuff (via union)
    // Doesn't contain any pointers
    // TODO: elaborate later
    char data1[0x7c];
    uint16_t stun_accumulator; // 7c
    uint16_t faint_countdown; // 7e
    char data2[7]; // 80
    uint8_t stun_resistance; // 87
    char data3[0xc0]; // 88
};
#pragma pack(pop)

static_assert(sizeof(gg_char_state) == 0x148);

struct player_button_timers
{
    // Doesn't contain any pointers
    // TODO: elaborate later
    char data[100];
};

struct player_direction_timers
{
    uint32_t timers[2];
#pragma pack(push, 1)
    struct timer
    {
        uint8_t dir;
        uint8_t timer;
    } dir_timers[2][32];
#pragma pack(pop)
    uint32_t dir_timer_idx[2];
};

struct camera_state
{
    // Doesn't contain any pointers
    // TODO: elaborate later
    char data[0xAC];
};

struct extra_config
{
    // Attack/defense/tension rate etc
    // Doesn't contain any pointers
    // TODO: elaborate later
    char data[0x98];
};

struct player_controller_state
{
    // Doesn't contain any pointers
    // TODO: elaborate later
    uint32_t sideswapped;
    char data1[8];
    uint32_t pressed;
    uint32_t depressed;
    char data2[132];
};

struct controller_state
{
    uint32_t bitmask_cur;
    uint32_t bitmask_prev;
    uint32_t bitmask_pressed_prev;
    uint32_t bitmask_pressed_cur;
    uint32_t bitmask_depressed;
    uint32_t timers[16];
    char unused[64];
    bool is_active;
};

static_assert(sizeof(controller_state) == 0x98);

struct mersenne_twister
{
    uint32_t index;
    uint32_t unknown;
    uint64_t data[624];
    bool twist_if_false;
};

static_assert(sizeof(mersenne_twister) == 5008);

struct sprite
{
};

struct hitbox
{
};


#pragma pack(push, 1)
template<size_t Size>
struct data_size
{
    uint8_t data[Size];
};
#pragma pack(pop)

struct active_object_state;
typedef int (palette_reset_func_t)(const active_object_state*);

#pragma pack(push, 1)
struct active_object_state
{
    uint16_t id; // 0
    uint8_t facing; // 2
    uint8_t side; // 3
    active_object_state* prev; // 4
    active_object_state* next; // 8
    uint32_t status_bitmask; // C
    uint32_t unknown_bitmask; // 10
    uint16_t active_move_prev; // 14
    uint16_t pad1; // 16
    uint16_t active_move; // 18
    uint16_t active_move_followup; // 1a
    uint16_t active_move_frame; // 1c
    uint16_t health; // 1e
    active_object_state* other_; // 20
    uint16_t last_jump_dir; // 24
    uint8_t unknown3; // 26
    uint8_t owner_id; // 27
    uint16_t unknown4; // 28
    uint16_t guard_status; // 2A
    gg_char_state* char_state_ptr; // 2C
    void* unknown_callback; // 30
    uint32_t unknown5[2]; // 34
    sprite* sprite_array_a; // 3C
    uint32_t sprite_array_b_idx; // 40
    uint32_t sprite_array_a_idx; // 44
    sprite* sprite_array_b; // 48
    uint32_t unknown6[2]; // 4C
    hitbox* hitbox_array; // 54
    uint32_t unknown7[3]; // 58
    active_object_state* other1; // 64
    uint32_t unknown8; // 68
    active_object_state* other2; // 6C
    uint8_t unknown9[0x14]; // 70
    uint8_t hitbox_count; // 84
    uint8_t unknown99[3]; // 85
    ptr_chain<data_size<0x5c>, 0, 0> data_5c1; // 88
    //data_5c* data_5c1; // 88
    ptr_chain<data_size<0x5c>, 0, 0> data_5c2; // 8C
    //data_5c* data_5c2; // 8C
    void* hit_block_callback; // 90
    palette_reset_func_t* reset_palette_callback; // 94
    uint8_t unknown10[0x14]; // 98
    uint32_t palette_status_bitmask; // ac
    int32_t pos_x; // b0
    int32_t pos_y; // b4
    int32_t velocity_x; // b8
    int32_t velocity_y; // bc
    uint32_t unknown11[8]; // c0
    void* unknown_ptr1; // e0
    uint32_t unknown12[2]; // e4
    void* unknown_ptr2; // ec
    void* unknown_ptr3; // f0
    uint32_t unknown13; // f4
    uint16_t unknown14; // f8
    uint16_t unknown15; // fa
    uint8_t unknown16; // fc
    uint8_t hitstop_countdown; // fd
    uint8_t unknown17[0x32]; // fe - 130
};
#pragma pack(pop)

//static_assert(sizeof(active_object_state) == 0x130);

struct projectiles
{
    active_object_state objects[0x80];
};

struct directx_obj
{
    ptr_chain<data_size<0x20>, 0, 0> vtable1;
    ptr_chain<data_size<0x20>, 0, 0> vtable2;
    directx_obj* ptr1;
    directx_obj* ptr2;
    uint32_t idx1;
    uint32_t idx2;
};

struct match_state
{
    memory_offset<uint8_t, 0x50f7ec> p1_rounds_won;
    memory_offset<uint8_t, 0x50f7ed> p2_rounds_won;
    memory_offset<uint32_t, 0x50f7fc> round_end_bitmask;
    memory_offset<uint16_t, 0x50F800> match_countdown;
    memory_offset<uint8_t, 0x5113C0> round_state;
    memory_offset<gg_char_state[2], 0x51A038> character_state;
    memory_offset<camera_state, 0x51B0D4> camera_state;
    memory_offset<player_button_timers, 0x516010> player_button_timers;
    memory_offset<player_direction_timers, 0x5161f8> player_direction_timers;
    memory_offset<player_controller_state[2], 0x516294> player_controller_state;
    memory_offset<uint8_t, 0x51B814> p1_selected_palette;
    memory_offset<uint8_t, 0x51B816> p2_selected_palette;
    // TODO: ac / gg / ggx
    memory_offset<bool, 0x51B8F8> p1_ex_enabled;
    memory_offset<bool, 0x51B8FC> p2_ex_enabled;
    // There are actually 4 controllers, but 3-4 are copies of 2
    memory_offset<controller_state[2], 0x51EDC8> controller_state;
    memory_offset<uint8_t[0x24], 0x519E50> extra_objects_meta;
    // TODO: optimize by capturing only objects in use
    // active_object_state[0x‭17F‬]
    memory_offset<ptr_chain<data_size<0x130 * 0x17F>, 0, 0>, 0x519E50> extra_objects;
    memory_offset<ptr_chain<active_object_state, 0, 0>, 0x516778> p1_character;
    memory_offset<ptr_chain<active_object_state, 0, 0>, 0x51A07C> p2_character;
    memory_offset<ptr_chain<projectiles, 0, 0>, 0x51677C> projectiles;

    memory_offset<uint8_t[0x120], 0x4FDC00> training_mode_history;
    memory_offset<uint8_t, 0x4FDD20> training_mode_cfg_display;
    // TODO: shorten / split this monstrosity
    memory_offset<uint8_t[0x7D5C], 0x4FDC00> training_mode_data;

    memory_offset<uint32_t, 0x555FEC> pause_state;

    memory_offset<uint8_t[0x90], 0x50AA0C> extra_rng_state;

    // TODO: try to shorten / remove some of this stuff
    memory_offset<uint8_t, 0x505A7D> graphics1;
    memory_offset<uint32_t, 0x506558> graphics2;
    memory_offset<uint32_t, 0x506588> graphics3;
    memory_offset<uint32_t, 0x506690> graphics4;
    memory_offset<uint32_t, 0x506694> graphics5;
    memory_offset<uint32_t, 0x506698> graphics6;
    memory_offset<uint32_t, 0x5066B0> graphics7;
    memory_offset<uint32_t, 0x5066B4> graphics8;
    memory_offset<uint8_t[0x8FA8], 0x521268> graphics9;
    memory_offset<uint32_t, 0x5476E8> graphics10;
    memory_offset<ptr_chain<data_size<0x19D78>, 0, 0>, 0x5480F0> graphics11;
    memory_offset<uint32_t, 0x548104> graphics12;
    memory_offset<uint32_t, 0x5489E0> graphics13;
    memory_offset<uint32_t, 0x5489F0> graphics14;
    memory_offset<uint32_t, 0x5489F8> graphics15;
    memory_offset<uint32_t, 0x55607C> graphics16;
    memory_offset<uint32_t, 0x55609C> graphics17;
    memory_offset<uint32_t, 0x5560A0> graphics18;
    memory_offset<uint8_t[0x1348], 0x548A00> graphics19;
    memory_offset<uint8_t[0x1CFC], 0x54B200> graphics20;
    
    // TODO: remove as much as possible of these 'unknown' values in the future
    memory_offset<uint32_t, 0x3E37FC> unknown1;
    memory_offset<uint32_t, 0x4F80E4> unknown2;
    memory_offset<uint32_t, 0x5113B4> unknown3;
    memory_offset<active_object_state, 0x517BA8> unknown4;
    memory_offset<uint32_t, 0x51B798> unknown5;
    memory_offset<uint32_t, 0x51B7A4> unknown6;
    memory_offset<uint32_t, 0x51B914> unknown7;
    memory_offset<uint32_t, 0x51B9DC> unknown8;
    memory_offset<uint32_t, 0x51EDD4> unknown9;
    memory_offset<uint32_t, 0x51EE6C> unknown10;
    memory_offset<uint32_t, 0x51EF04> unknown11;
    memory_offset<uint32_t, 0x51EF9C> unknown12;
    memory_offset<uint32_t, 0x555D28> unknown13;
    memory_offset<uint32_t, 0x55602C> unknown14;
    memory_offset<uint32_t, 0x5561A8> unknown15;
    memory_offset<active_object_state, 0x517A78> unknown16;
    memory_offset<uint8_t[0x2800], 0x5489F8> unknown17;
    memory_offset<uint8_t[0x2800], 0x54B198> unknown18;
    memory_offset<uint8_t[0x54], 0x506690> unknown19;
    memory_offset<ptr_chain<data_size<0x1CFF0>, 0, 0>, 0x5066A4> unknown20;
    memory_offset<ptr_chain<data_size<0x1CFF0>, 0, 0>, 0x5066A8> unknown21;
    memory_offset<active_object_state, 0x5163E0> unknown22;
};

static_assert(sizeof(player_controller_state) == 152);
static_assert(sizeof(match_state().player_direction_timers.get()) == 144);

#pragma pack(push, 1)
struct game_config
{
    struct controller_config
    {
        struct bitmask
        {
            uint16_t bit;
            uint16_t pad;
        };
        static constexpr bitmask up = {0x10};
        static constexpr bitmask down = {0x40};
        static constexpr bitmask left = {0x80};
        static constexpr bitmask right = {0x20};
        bitmask p; // 0
        bitmask k; // 4
        bitmask s; // 8
        bitmask hs; // c
        bitmask d; // 10
        bitmask taunt; // 14
        bitmask reset; // 18
        bitmask pause; // 1c
        bitmask rec_player; // 20
        bitmask rec_enemy; // 24
        bitmask play_memory; // 28
        bitmask switch_control; // 2c
        bitmask enemy_walk; //30
        bitmask enemy_jump; // 34
        bitmask pk; // 38
        bitmask pd; // 3c
        bitmask pks; // 40
        bitmask pksh; // 44
        // 1-4 presets, 0=controller, 5=custom
        uint32_t mode_id; // 48
        uint32_t unknown2; // 4c
    };
    controller_config player_controller_config[2];
    controller_config controller_config_presets[4];

    uint8_t unknown[0x2948];
};
#pragma pack(pop)

static_assert(sizeof(game_config::controller_config) == 0x50);
static_assert(sizeof(game_config) == 0x2b28);

uint16_t reverse_bytes(uint16_t value);

enum class fiber_id : uint32_t
{
    none1 = 0,
    none2 = 0xa,
    title = 9,
    movie = 45,
    main_menu = 0x19,
    charselect = 0xf
};

enum class main_menu_idx : uint32_t
{
    training = 7
};

struct gg_state
{
    // TODO: memory_offset => external
    // TODO: remove rel_mem_ptr altogether
    memory_offset<rel_mem_ptr<decltype(game_tick)>, 0x14756E + 1> game_tick;
    memory_offset<rel_mem_ptr<decltype(process_input)>, 0x146F95 + 1> process_input;
    memory_offset<rel_mem_ptr<decltype(get_raw_input_data)>, 0x5263d + 1> get_raw_input_data;
    memory_offset<rel_mem_ptr<decltype(process_objects)>, 0x146F9A + 1> process_objects;
    memory_offset<rel_mem_ptr<decltype(limit_fps)>, 0x14ADC2 + 1> limit_fps;
    memory_offset<decltype(sleep)**, 0x14770C + 2> sleep_ptr;
    //**(*(*(:base+0x556020)+0x58)+0x34)+0x14
    memory_offset<
        ptr_chain<decltype(play_sound)*, 0, 0x58, 0x34, 0, 0x14>,
        0x556020> play_sound;
    // TODO: how to populate it automatically?
    write_cockpit_font_func_t* write_cockpit_font = nullptr;
    write_pretty_font_func_t* write_pretty_font = nullptr;
    write_utf8_font_func_t* write_utf8_font = nullptr;
    draw_rect_func_t* draw_rect = nullptr;
    draw_pressed_buttons_func_t* draw_pressed_buttons = nullptr;
    remap_action_buttons_func_t* remap_action_buttons = nullptr;
    remap_direction_buttons_func_t* remap_direction_buttons = nullptr;
    draw_arrow_func_t* draw_arrow = nullptr;
    decltype(player_status_ticker)* player_status_ticker = nullptr;
    memory_offset<IDirect3DDevice9**, 0x555B94> direct3d9;
    memory_offset<extra_config, 0x51B180> extra_config[2];
    // TODO: at least 14! Double check
    memory_offset<menu_fiber[14], 0x54f030> menu_fibers;
    memory_offset<mersenne_twister, 0x565F20> rng;
    /*
    vs 2p: 0x803
    training: 0x101
    vp cpu: 0x2001
    vs 2p team: 0x40803
    vs cpu team: 0x42001
    arcade/mom: 1
    survival 0x201
    */
    memory_offset<uint32_t, 0x51B8CC> game_mode;
    memory_offset<game_config, 0x5134D8> config;
    memory_offset<fiber_id, 0x555FF4> next_fiber_id;
    memory_offset<bool, 0x5136C4> skip_saving;
    memory_offset<bool, 0x50BF30> charselect_p1_enabled;
    memory_offset<bool, 0x50BF68> charselect_p2_enabled;
    memory_offset<main_menu_idx, 0x5557B0> main_menu_idx;
};

extern gg_state g_state;
extern gg_state g_state_orig;

void EnableDrawing(bool enable);

void EnableRoundEndCondition(bool enable);

// untested, but should work
void EnablePauseMenu(bool enable);

namespace memory_hook
{

extern bool g_capture;
HLOCAL WINAPI LocalFree(HLOCAL hMem);
HLOCAL WINAPI LocalAlloc(UINT uFlags, SIZE_T uBytes);

}

extern bool g_enable_fps_limit;

void apply_patches(char* image_base);

extern "C" __declspec(dllexport) void libgg_init();

bool in_match();

void queue_destroy_fibers();

using history_t = std::tuple<
    match_state,
    decltype(g_state.rng),
    input_data
>;

// set palette reset bit
// current palette may be incorrect after rollback
// (lightning / fire effect)
// call this function after any kind of time travel
void set_pallette_reset_bit(history_t& state);

// rec player = rec / stop recording
// rec enemy = stop world
// play memory = play / stop playing
// enemy walk = reverse, twice = max speed reverse
// enemy jump = forward, twice = max speed forward
// reset = reset current input
void __cdecl get_raw_input_data(input_data* out);

int32_t limit_fps();

void game_tick();

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

int32_t __stdcall play_sound(IXACT3WaveBank* a1, int16_t a2, uint32_t a3, int32_t a4, int8_t a5, IXACT3Wave** a6);

void player_status_ticker(const char* message, uint32_t side);

void process_input();

template<size_t N, std::enable_if_t<(N > 1)>* = nullptr>
std::pair<char*, std::errc> format_int(char (&buffer)[N], int value, int pad = 3, char pad_c = ' ')
{
    auto [p, ec] = std::to_chars(buffer, buffer + N - 1, value);
    if (ec != std::errc())
        return {nullptr, ec};
    char* end = p;
    const size_t len = end - buffer;
    if (len < pad)
    {
        const size_t diff = pad - len;
        end += diff;
        for (int i = pad - 1; i >= 0; --i)
        {
            if (i < diff)
                buffer[i] = pad_c;
            else
                buffer[i] = buffer[i - diff];
        }
    }
    *end = 0;

    return {end, std::errc()};
}

void write_utf8_font(const char* text, int x, int y,
                     float z, float opacity,
                     uint32_t unknown3, uint32_t unknown4,
                     float scale_x = 1, float scale_y = 1);

void draw_rect(uint32_t color, uint32_t x1, uint32_t y1,
               uint32_t x2, uint32_t y2, uint32_t unknown);

uint32_t remap_action_buttons(uint32_t input, const active_object_state* obj);

uint32_t remap_direction_buttons(uint32_t input);

void process_objects();
