#pragma once

#include "fiber_mgmt.h"
#include "memory_dump.h"

#include <array>
#include <memory>
#include <utility>


using memory_dump::ptr_chain;
using memory_dump::rel_mem_ptr;
using memory_dump::memory_offset;
using memory_dump::offset_value;


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
    }; // 50
    controller_config player_controller_config[2];
    controller_config controller_config_presets[4];
    static constexpr controller_config default_controller_config = {
        {0x8000}, // p
        {0x4000}, // k
        {0x1000}, // s
        {0x2000}, // hs
        {0x800}, // d
        {0x200}, // taunt
        {1}, // reset
        {8}, // pause
        {0}, // rec player
        {0x400}, // rec enemy
        {0x100}, // play memory
        {0}, // switch
        {2}, // enemy walk
        {4}, // enemy jump
        {0}, // pk
        {0}, // pd
        {0}, // pks
        {0} // pksh
    };

    uint8_t unknown1[0xC]; // 1E0
    uint32_t skip_saving; // 1EC
    uint8_t unknown2[0x2938]; // 1F0
};
#pragma pack(pop)

static_assert(sizeof(game_config::controller_config) == 0x50);
static_assert(sizeof(game_config) == 0x2b28);

struct input_data
{
    uint16_t keys[2];
    struct joystick
    {
        uint8_t axes[4];
    } joysticks[2];
    uint32_t is_active[2];
    uint32_t unknown[2];
};

static_assert(sizeof(input_data) == 0x1c);

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
// common state + char-specific stuff (via union)
struct gg_char_state
{
    uint16_t tension; // 0
    char data1[0x18]; // 2
    int16_t guard; // 1a
    char data2[0x60]; // 1c
    uint16_t stun_accumulator; // 7c
    uint16_t faint_countdown; // 7e
    char data3[7]; // 80
    uint8_t stun_resistance; // 87
    char data4[0x70]; // 88
    uint16_t burst; // f8
    char data5[0x4e]; // fa
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
    const gg_char_state* char_state_ptr; // 2C
    void* unknown_callback; // 30
    uint32_t unknown5[2]; // 34
    void* sprite_array_a; // 3C
    uint32_t sprite_array_b_idx; // 40
    uint32_t sprite_array_a_idx; // 44
    void* sprite_array_b; // 48
    uint32_t unknown6[2]; // 4C
    void* hitbox_array; // 54
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

enum class fiber_id : uint32_t
{
    none1 = 0,
    none2 = 0xa,
    title = 9,
    movie = 45,
    main_menu = 0x19,
    charselect = 0xf,
    match = 6
};

struct match_state
{
    memory_offset<gg_char_state[2], 0x51A038> character_state;
    memory_offset<camera_state, 0x51B0D4> camera_state;
    memory_offset<player_button_timers, 0x516010> player_button_timers;
    memory_offset<player_direction_timers, 0x5161f8> player_direction_timers;
    memory_offset<player_controller_state[2], 0x516294> player_controller_state;
    #pragma pack(push, 1)
    struct data_
    {
        uint8_t selected_palette[2]; // 0
        uint8_t unknown1[0x12]; // 2
        uint16_t selected_char[2]; // 14
        uint8_t unknown2[0xe]; // 18
        uint16_t winner; // 26
        uint8_t unknown3[0x6]; // 28
        uint16_t winstreak; // 2e
    };
    #pragma pack(pop)
    static_assert(sizeof(data_) == 0x30);
    memory_offset<data_, 0x51B814> data;
    memory_offset<uint32_t[2], 0x51B8F0> char_mode_gold;
    memory_offset<uint32_t[2], 0x51B8F8> char_mode_ex;
    memory_offset<uint32_t[2], 0x51B900> char_mode_sp;
    // There are actually 4 controllers, but 3-4 are copies of 2
    memory_offset<controller_state[2], 0x51EDC8> controller_state;
    // not sure if this is used outside of built-in netplay
    memory_offset<::controller_state[2], 0x51E968> controller_state2;
    memory_offset<uint8_t[0x24], 0x519E50> extra_objects_meta;
    // TODO: optimize by capturing only objects in use
    memory_offset<ptr_chain<data_size<0x130 * 0x17f>, 0, 0>, 0x519E50> extra_objects;
    memory_offset<active_object_state*, 0x516778> p1_character_ptr;
    memory_offset<active_object_state*, 0x51A07C> p2_character_ptr;
    memory_offset<ptr_chain<active_object_state, 0, 0>, 0x516778> p1_character;
    memory_offset<ptr_chain<active_object_state, 0, 0x130>, 0x516778> p2_character;
    // active_object_state[0x80]
    memory_offset<ptr_chain<data_size<0x130 * 0x80>, 0, 0>, 0x51677C> projectiles;

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

    // Used for validating throw attempt (opponent X axis position?).
    // If it's not stored in game state, then rollback to the first frame
    // of throws that put opponent far away (Kliff) will result in a hit
    // instead of throw.
    // test: test/replays/bugrepro/kliff_throw.ggr
    memory_offset<uint32_t, 0x51B78C> unknown23;

    // When a clash happens, this address is used as a condition
    // for some calculations involving RNG (:base+6404A)
    // test: test/replays/bugrepro/clash.ggr
    memory_offset<uint32_t, 0x51B0CC> unknown24;
};

static_assert(sizeof(player_controller_state) == 152);
static_assert(sizeof(match_state().player_direction_timers.get()) == 144);

struct fiber_state
{
    // FIN
    memory_offset<uint32_t, 0x55606c> pause_state;
    // FIN
    memory_offset<uint32_t, 0x5202e0> data1;
    // FIN
    memory_offset<float, 0x5202e4> data2;
    // FIN, NXBT
    memory_offset<uint32_t, 0x5202e8> data3;

    memory_offset<uint32_t[0x70], 0x50BF30> charselect1;
    memory_offset<uint32_t[0x72], 0x50ACC4> charselect2;
    memory_offset<uint32_t[0x36], 0x555C40> charselect3;
    memory_offset<uint32_t[2], 0x520DD0> charselect4;
    memory_offset<uint32_t, 0x520E7C> charselect5;
    memory_offset<uint32_t, 0x520E9C> charselect6;
    memory_offset<uint32_t, 0x520EBC> charselect7;
    memory_offset<uint32_t, 0x520EDC> charselect8;
    memory_offset<uint32_t[2], 0x50AE38> charselect9;
    memory_offset<uint32_t[0x1d], 0x44E3E0> charselect10;
    memory_offset<uint32_t, 0x51B9E4> charselect11;
    memory_offset<uint32_t, 0x3EA9FC> stage_select_controller;
    memory_offset<uint64_t[0x10a], 0x44E664> random_stage_sequence;
    memory_offset<uint64_t[6], 0x44E454> random_char_sequence;

    // FOUT
    memory_offset<uint32_t, 0x511B40> fout_condition;

    // XAudio file loading queue?
    memory_offset<uint32_t[0x1a], 0x51BA2C> data4;
    memory_offset<uint32_t[0x82], 0x520B40> data5;

    // VSCHA, VSVS ...
    memory_offset<uint32_t[0x26], 0x555E6C> data6;
};

struct memory_regions
{
    // :base+41F70 allocates a memory region of 3800000 bytes (58 Mb)
    // that is used extensively throughout process lifetime.
    // Region address is stored at :base+54810C.
    // It's divided into subregions of following sizes:
    // 5A4800 500000 800000 800000 600000 190000 100000
    // 100000 20000 20000 A000 100000 400000 800000 600000 81800
    // 
    // "GINI" and "VS  " fibers access the region of size 600000
    // through a pointer at :base+0x520C1C.
    // "VS  ": modifies region via ReadFile() in background thread
    //         reads inside :base+4B7D0
    // "GINI": modifies region memory directly
    //         reads at (not important)
    // For correct rollback, whole region should be restored (600000).
    // It's obviously too big for that, so let's cheat for now
    // and save only the "observed" size (i.e. non-zero memory)
    // TODO: figure out how to rollback large memory regions in a fast and correct manner
    std::shared_ptr<std::array<uint64_t, 0x1471E>> region1;
};

// _tiddata layout:
// * https://github.com/nicecoolwinter/learn_c/blob/aa31897008b3042f9e49f52beee21dd5ba7a5ec6/vc_lib_src/src/mtdll.h#L135
// * we're only interested in 'unsigned long   _holdrand' (offset 0x14)
struct _tiddata
{
    uint32_t _dontcare[5]; // 0
    uint32_t _holdrand; // 14
};

// match_state split into two types:
// cl.exe cannot handle so much templates and stops with an out of memory error.
struct match_state_2
{
    memory_offset<uint32_t, 0x51B914> frame;
    memory_offset<uint8_t, 0x50F7e8> round_end_flag1;
    memory_offset<uint8_t, 0x50f7ec> p1_rounds_won;
    memory_offset<uint8_t, 0x50f7ed> p2_rounds_won;
    memory_offset<uint32_t, 0x50f7fc> round_end_bitmask;
    memory_offset<uint32_t, 0x50F7F4> round_end_flag2;
    memory_offset<uint16_t, 0x50F800> match_countdown;
    memory_offset<uint32_t, 0x50F804> round_end_flag3;
    memory_offset<uint32_t, 0x555D24> round_end_hitstop;
    memory_offset<uint8_t, 0x5113C0> round_state;
    memory_offset<menu_fiber[0x20], 0x54f030> menu_fibers;
    memory_offset<mersenne_twister, 0x565F20> rng1;
    // not sure if needed
    memory_offset<mersenne_twister, 0x564B60> rng2;
    memory_offset<extra_config, 0x51B180> extra_config[2];
    /*
    vs 2p: 0x803
    training: 0x101 / 0x102
    vp cpu: 0x2001 / 0x2002
    vs 2p team: 0x40803
    vs cpu team: 0x42001 / 0x 42002
    arcade/mom: 1 / 2
    survival 0x201 / 0x202
    */
    memory_offset<uint32_t, 0x51B8CC> game_mode;
    memory_offset<game_config, 0x5134D8> config;
    memory_offset<fiber_id, 0x555FF4> next_fiber_id;
    memory_offset<bool, 0x50BF30> charselect_p1_enabled;
    memory_offset<bool, 0x50BF68> charselect_p2_enabled;
    memory_offset<uint32_t, 0x5557B0> main_menu_idx;

    // some of this data is required to avoid rng desync during rollback at
    // the beginning of fafnir -> tyrant rave (flashy effect uses rng)
    // test: test/replays/bugrepro/fafnir_tyrant_rave.ggr
    memory_offset<float[4], 0x50F814> effect_data1;
    memory_offset<uint32_t, 0x50F824> effect_data2;
    memory_offset<uint32_t[3], 0x511224> effect_data3;
    memory_offset<uint32_t, 0x51B820> effect_data4;
    memory_offset<float, 0x5113B0> effect_data5;
    memory_offset<uint32_t, 0x520DCC> effect_data6;
    memory_offset<data_size<0x48>, 0x5087A8> effect_data7;

    // Address of fiber-local storage index for _tiddata structure.
    // _tiddata contains rand() seed. Kliff's taunt (バカモン) employs
    // rand() instead of Mersenne Twister (strange choice).
    // rand() implementation:
    // * https://github.com/nicecoolwinter/learn_c/blob/aa31897008b3042f9e49f52beee21dd5ba7a5ec6/vc_lib_src/src/rand.c#L58
    memory_offset<uint32_t, 0x3C7D04> tiddata_fls_index;

    // _tiddata::_holdrand
    uint32_t rand_seed;

    // Used by FIN fiber (fades the screen to black
    // when SELECT is pressed in training mode)
    memory_offset<float, 0x51f22c> black_screen_opacity;

    memory_offset<uint32_t, 0x3EACDC> selected_bgm;
    memory_offset<uint32_t, 0x516048> selected_stage;

    memory_offset<uint32_t, 0x567584> slayer_haiku;

    // test: test/replays/matches/session3.ggr
    // at :base+7E600
    memory_offset<uint64_t[0x200], 0x512310> round_end_rng_related1;
};

struct game_state
{
    match_state match;
    match_state_2 match2;
    std::vector<fiber_mgmt::fiber_state> fibers;
    fiber_state fiber_state;
    memory_regions regions;
};

typedef void (process_input_func_t)();
typedef void (process_objects_func_t)();
typedef void (get_raw_input_data_func_t)(input_data* out);
typedef int32_t (limit_fps_func_t)();
typedef void (game_tick_func_t)();
typedef void (wait_file_readers_func_t)();
typedef void (__stdcall sleep_func_t)(uint32_t ms);
struct IXACT3WaveBank;
struct IXACT3Wave;
// IXACT3WaveBank_Play(__in IXACT3WaveBank* pWaveBank, XACTINDEX nWaveIndex, DWORD dwFlags, DWORD dwPlayOffset, XACTLOOPCOUNT nLoopCount, __deref_out IXACT3Wave** ppWave)
typedef int32_t (__stdcall play_sound_func_t)(IXACT3WaveBank*, int16_t, uint32_t, int32_t, int8_t, IXACT3Wave**);
typedef int (write_cockpit_font_func_t)(const char* buffer, int x, int y, float z, uint8_t alpha, float scale);
typedef void (player_status_ticker_func_t)(const char* message, uint32_t side);
// Example arguments:
// TRAINING MENU, 42000000, 42200000, 40000000, 1, 5, 3f800000
// PLAYER, 42800000, 42A00000, 40000000, 1, 5, 3F800000
// ENEMY, 43C00000, 42A00000, 40000000, A0, 5, 3F800000
// H-SLASH, 42000000, 43480000, 40000000,A0, 7, 3F800000
// SWITCH, 43830000, 42C00000, 40800000, 1, 6, 3F800000
// ver R , 44070000 , 43A30000, 41880000, 1, 0x11, 3F000000
typedef void (write_special_font_func_t)(
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
typedef void (write_utf8_font_func_t)(
    int x, int y, float z, float opacity, uint32_t unknown3, uint32_t unknown4
);
// Color in EAX: 0xAARRGGBB
// TODO: specify value for 'unknown'
typedef void (draw_rect_func_t)(
    uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t unknown
);
// unknown1 = 3, unknown2 = 1
// process_objects() deletes previously drawn buttons
// input: first byte = direction bitmask, second byte = action bitmask
// input must not contain both directions and actions
// split input and call draw_pressed_buttons_func_t two times if needed
typedef void (draw_pressed_buttons_func_t)(
    uint32_t input, uint32_t x, uint32_t y, uint32_t unknown1, uint32_t unknown2
);
// Remap action buttons according to player's controller config before
// displaying them via draw_pressed_buttons_func_t.
// mode: 0 (controller), 1-4 (presets). Value doesn't really matter,
// as long as it's not 0. Passing 0 results in incorrect button mapping.
// This is due to a bug in GG: set MODE to CONTROLLER and DISPLAY to INPUT,
// displayed buttons will be incorrect.
// input registers:
// * ecx: copy of input
// * edx: active_object_state* for current player
// output registers:
// * eax: result
typedef uint32_t (button_bitmask_to_icon_bitmask_func_t)(uint32_t mode);
// This function is doing some useless bit-swapping, but it's required for
// draw_pressed_buttons_func_t.
// unknown = 0
// input registers:
// * ecx: copy of input
// output registers:
// * eax: result
typedef uint32_t (direction_bitmask_to_icon_id_func_t)(uint32_t input, uint32_t unknown);
// Draw an arrow icon. For example, DISPLAY option in training mode pause menu.
// unknown = 2
typedef void (draw_arrow_func_t)(
    uint32_t arrow_type, uint32_t x, uint32_t y, uint32_t unknown, uint32_t alpha
);

struct gg_state
{
    // TODO: memory_offset => external
    // TODO: remove rel_mem_ptr altogether
    memory_offset<rel_mem_ptr<game_tick_func_t>, 0x14756E + 1> game_tick;
    memory_offset<rel_mem_ptr<process_input_func_t>, 0x146F95 + 1> process_input;
    memory_offset<rel_mem_ptr<get_raw_input_data_func_t>, 0x5263d + 1> get_raw_input_data;
    memory_offset<rel_mem_ptr<process_objects_func_t>, 0x146F9A + 1> process_objects;
    memory_offset<rel_mem_ptr<limit_fps_func_t>, 0x14ADC2 + 1> limit_fps;
    memory_offset<sleep_func_t**, 0x14770C + 2> sleep_ptr;
    //**(*(*(:base+0x556020)+0x58)+0x34)+0x14
    memory_offset<
        ptr_chain<play_sound_func_t*, 0, 0x58, 0x34, 0, 0x14>,
        0x556020> play_sound;
    // TODO: how to populate it automatically?
    write_cockpit_font_func_t* write_cockpit_font = nullptr;
    write_special_font_func_t* write_special_font = nullptr;
    write_utf8_font_func_t* write_utf8_font = nullptr;
    draw_rect_func_t* draw_rect = nullptr;
    draw_pressed_buttons_func_t* draw_pressed_buttons = nullptr;
    button_bitmask_to_icon_bitmask_func_t* button_bitmask_to_icon_bitmask = nullptr;
    direction_bitmask_to_icon_id_func_t* direction_bitmask_to_icon_id = nullptr;
    draw_arrow_func_t* draw_arrow = nullptr;
    player_status_ticker_func_t* player_status_ticker = nullptr;
    wait_file_readers_func_t* wait_file_readers = nullptr;

    struct IDirect3DDevice9;
    memory_offset<IDirect3DDevice9**, 0x555B94> direct3d9;
    memory_offset<HWND, 0x506554> hwnd;
};

void load_global_data(size_t memory_base, gg_state& state);
void dump_global_data(size_t memory_base, const gg_state& state);

void revert_state(size_t image_base, game_state& state, fiber_mgmt::fiber_service* service = nullptr);
void save_current_state(size_t image_base, game_state& state, fiber_mgmt::fiber_service* service = nullptr);
uint32_t state_checksum(const game_state& state);
void print_game_state(const game_state& state);
