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

char* g_image_base = 0;

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
const auto sleep_ptr = &sleep;
int __cdecl write_cockpit_font_internal(int x, int y, float z, uint8_t alpha, float scale);
// Wrapper for write_cockpit_font_internal
int write_cockpit_font(const char* buffer, int x, int y, float z, uint8_t alpha, float scale);
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
	char data2[0xc8];
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

//static_assert(sizeof(active_object_state) == 0x130);

struct projectiles
{
	active_object_state objects[0x80];
};

template<>
struct reflect<projectiles>
{
	constexpr static auto members = member_tuple(
		&projectiles::objects
	);
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

	// TODO: shorten / split this monstrosity
	memory_offset<uint8_t[0x7E7D], 0x4FDC00> training_mode_history;

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
		// 1-4, 0=controller, 5=custom
		uint32_t preset_id; // 48
		uint32_t unknown2; // 4c
	};
	controller_config player_controller_config[2];
	controller_config controller_config_presets[4];

	uint8_t unknown[0x2948];
};
#pragma pack(pop)

static_assert(sizeof(game_config::controller_config) == 0x50);
static_assert(sizeof(game_config) == 0x2b28);

uint16_t reverse_bytes(uint16_t value)
{
	return value / 256 + (value & 0xff) * 256;
}

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
	memory_offset<decltype(&sleep_ptr), 0x14770C + 2> sleep_ptr;
	//**(*(*(:base+0x556020)+0x58)+0x34)+0x14
	memory_offset<
		ptr_chain<decltype(play_sound)*, 0, 0x58, 0x34, 0, 0x14>,
		0x556020> play_sound;
	// TODO: how to populate it automatically?
	decltype(write_cockpit_font_internal)* write_cockpit_font = nullptr;
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
	memory_offset<uint8_t*, 0x54EE14> memory_begin;
	memory_offset<uint8_t*, 0x54B208> memory_end;
	memory_offset<game_config, 0x5134D8> config;
	memory_offset<fiber_id, 0x555FF4> next_fiber_id;
	memory_offset<bool, 0x5136C4> skip_saving;
	memory_offset<bool, 0x50BF30> charselect_p1_enabled;
	memory_offset<bool, 0x50BF68> charselect_p2_enabled;
	memory_offset<main_menu_idx, 0x5557B0> main_menu_idx;
} g_state, g_state_orig;

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
		&gg_state::memory_begin,
		&gg_state::memory_end,
		&gg_state::config,
		&gg_state::next_fiber_id,
		&gg_state::skip_saving,
		&gg_state::charselect_p1_enabled,
		&gg_state::charselect_p2_enabled,
		&gg_state::main_menu_idx
	);
};

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

extern "C" __declspec(dllexport) void libgg_init()
{
	static bool s_is_ready = false;
	if (!s_is_ready)
	{
		g_image_base = (char*)::GetModuleHandle(nullptr);
		load(g_image_base, g_state);
		g_state.write_cockpit_font = reinterpret_cast<decltype(write_cockpit_font_internal)*>(g_image_base + 0x10E530);
		g_state.player_status_ticker = reinterpret_cast<decltype(player_status_ticker)*>(g_image_base + 0x10E190);
		g_state_orig = g_state;
		s_is_ready = true;
		g_state.get_raw_input_data.get().set(get_raw_input_data);
		g_state.limit_fps.get().set(limit_fps);
		g_state.game_tick.get().set(game_tick);
		g_state.process_input.get().set(process_input);
		g_state.process_objects.get().set(process_objects);
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

using history_t = std::tuple<
	match_state,
	decltype(g_state.rng),
	std::vector<uint8_t>,
	input_data
>;
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
struct extra_training_display
{
	uint16_t stun_accumulator;
	uint16_t faint_countdown;
} g_extra_training_display;

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
	load(g_image_base, g_state.game_mode);

	load(g_image_base, std::get<0>(g_cur_state));

	auto& p1_char_optional = std::get<0>(g_cur_state).p1_character.get().ptr;
	if (in_match() && p1_char_optional)
	{
		//memory_hook::g_capture = true;

		if (g_speed <= 0)
		{
			g_cur_state = *g_prev_state;
			set_pallette_reset_bit(g_cur_state);
			const auto& ms = std::get<0>(g_cur_state);
			const auto& rng = std::get<1>(g_cur_state);
			const auto& data1 = std::get<2>(g_cur_state);
			std::copy(data1.begin(), data1.end(), g_state.memory_begin.get());
			dump(ms, g_image_base);
			dump(rng, g_image_base);

			//free_orphaned_allocations(memory_hook::g_allocations);
			//memory_hook::g_allocations.clear();
		}
		else
		{
			load(g_image_base, std::get<1>(g_cur_state));
			std::get<2>(g_cur_state).clear();
			std::copy(g_state.memory_begin.get(), g_state.memory_end.get(),
						std::back_inserter(std::get<2>(g_cur_state)));
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
					const auto& prev_keys = std::get<3>(*g_prev_state).keys[i];
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

			if (bitmask & cfg.enemy_jump.bit)
			{
				speed_control_enabled = true;
				if (!g_manual_frame_advance)
				{
					g_speed = 2;
				}
				else
				{
					if (!(g_prev_bitmask[i] & cfg.enemy_jump.bit) || g_speed_control_counter > 60)
					{
						g_speed = 1;
					}
					else
					{
						g_speed = 0;
					}
				}
				++g_speed_control_counter;
			}

			if (bitmask & cfg.enemy_walk.bit)
			{
				speed_control_enabled = true;
				if (!g_manual_frame_advance)
				{
					g_speed = 0;
				}
				else
				{
					if (!(g_prev_bitmask[i] & cfg.enemy_walk.bit) || g_speed_control_counter > 60)
					{
						g_speed = -1;
					}
					else
					{
						g_speed = 0;
					}
				}
				++g_speed_control_counter;
			}

			g_prev_bitmask[i] = bitmask;
		}

		if (!speed_control_enabled)
		{
			if (g_manual_frame_advance)
				g_speed = 0;
			else
				g_speed = 1;
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

		std::get<3>(g_cur_state) = input;

		if (g_playing)
		{
			if (g_playback_idx < g_history.size())
			{
				g_cur_state = g_history[g_playback_idx];
				input = std::get<3>(g_cur_state);
				if (g_playback_idx == 0)
				{
					set_pallette_reset_bit(g_cur_state);
					const auto& ms = std::get<0>(g_cur_state);
					const auto& rng = std::get<1>(g_cur_state);
					const auto& data1 = std::get<2>(g_cur_state);
					std::copy(data1.begin(), data1.end(), g_state.memory_begin.get());
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

		if (g_recording && g_record_idx < 900)
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

		if (g_state.game_mode == 0x101)
		{
			// in training
			const auto& p2_char_state = std::get<0>(g_cur_state).character_state.get()[1];
			g_extra_training_display.stun_accumulator = p2_char_state.stun_accumulator;
			g_extra_training_display.faint_countdown = p2_char_state.faint_countdown;
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

	if (g_state.memory_begin.get() == g_state.memory_end.get())
	{
		load(g_image_base, g_state.memory_begin);
		load(g_image_base, g_state.memory_end);
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

int write_cockpit_font(const char* buffer, int x, int y, float z, uint8_t alpha, float scale)
{
	auto f = g_state.write_cockpit_font;
	uint32_t alpha_ = alpha;
	// Custom calling convention due to LTCG:
	// * First argument in EAX
	// * Cleanup by the caller
	__asm
	{
		push scale
		push alpha_
		push z
		push y
		push x
		mov eax, buffer
		call f
		add esp, 4*5
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

template<size_t N, std::enable_if_t<(N > 1)>* = nullptr>
std::errc format_int(char (&buffer)[N], int value, int pad = 3)
{
	auto [p, ec] = std::to_chars(buffer, buffer + N - 1, value);
	if (ec != std::errc())
		return ec;
	const size_t len = p - buffer;
	if (len < pad)
	{
		const size_t diff = pad - len;
		buffer[pad] = 0;
		for (int i = pad - 1; i >= 0; --i)
		{
			if (i < diff)
				buffer[i] = ' ';
			else
				buffer[i] = buffer[i - diff];
		}
	}
	else
	{
		*p = 0;
	}
	return std::errc();
}

void process_objects()
{
	const auto f = *g_state_orig.process_objects.get().get();
	f();
	write_cockpit_font("DEV BUILD", 50, 100, 1, 0x50, 1);
	if (g_recording)
	{
		auto str = "REC " + std::to_string(g_record_idx);
		write_cockpit_font(str.c_str(), 285, 100, 1, 0xFF, 1);
	}
	if (g_playing)
	{
		auto str = "PLAY " + std::to_string(g_playback_idx);
		write_cockpit_font(str.c_str(), 285, 100, 1, 0xFF, 1);
	}
	if (g_speed != 1 || g_manual_frame_advance)
	{
		auto str = "SPEED " + std::to_string(g_speed);
		write_cockpit_font(str.c_str(), 285, 150, 1, 0xFF, 1);
	}
	if (g_out_of_memory)
	{
		write_cockpit_font("OUT OF MEMORY!", 50, 150, 1, 0xff, 1);
	}
	if (in_match() && g_state.game_mode == 0x101 && !std::get<0>(g_cur_state).pause_state.get())
	{
		// TODO: don't display extra HUD if DISPLAY option is NONE or INPUT
		const int increment_y = 0x18;
		const int key_x = 0x16a;
		const int value_x = 0x21a;
		const float key_z = 265;
		const float value_z = 266;
		int y = 0xb8;
		{
			y += increment_y;
			write_cockpit_font("        STUN", key_x, y, key_z, 0xff, 1);
			if (g_extra_training_display.stun_accumulator != 0xffff)
			{
				char str[6];
				format_int(str, g_extra_training_display.stun_accumulator);
				write_cockpit_font(str, value_x, y, value_z, 0xff, 1);
			}
			else
			{
				write_cockpit_font("FAINT", value_x, y, value_z, 0xff, 1);
			}

			if (g_extra_training_display.faint_countdown)
			{
				y += increment_y;
				write_cockpit_font("       FAINT", key_x, y, key_z, 0xff, 1);
				{
					char str[6];
					format_int(str, g_extra_training_display.faint_countdown);
					write_cockpit_font(str, value_x, y, value_z, 0xff, 1);
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
