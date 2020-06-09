#include "game.h"

#include "memory_dump.h"
#include "util.h"
#include "xact_audio.h"

#include <chrono>
#include <unordered_map>
#include <unordered_set>
#include <vector>


using memory_dump::load;
using memory_dump::dump;
using memory_dump::dump_unprotected;
using memory_dump::local_memory_accessor;

int32_t play_sound_impl(IXACT3WaveBank*, int16_t, uint32_t, int32_t, int8_t, IXACT3Wave**);
void gg_main_loop_impl();

// hooks for game functions
namespace
{

IGame* g_game = nullptr;
fiber_mgmt::fiber_service::ptr_t g_fiber_service;
XACT3Wave g_empty_wave;

int32_t __stdcall play_sound(IXACT3WaveBank* a1, int16_t a2, uint32_t a3, int32_t a4, int8_t a5, IXACT3Wave** a6)
{
    return play_sound_impl(a1, a2, a3, a4, a5, a6);
}

void gg_main_loop()
{
    gg_main_loop_impl();
}

void get_input_from_cache(input_data* out)
{
    auto input = g_game->GetCachedInput();
    *out = input_data{};
    out->is_active[0] = 1;
    out->is_active[1] = 1;
    out->keys[0] = input[0];
    out->keys[1] = input[1];
}

uint32_t get_current_fps()
{
    return g_game->GetCurrentFps();
}

}

class Game : public IGame
{
public:
    Game(size_t image_base, configuration*) : m_image_base(image_base)
    {
        load_global_data(image_base, m_globals_orig);

        PatchMainLoop();

        // Each frame, RNG (Mersenne Twister) index is incremented even if it was unused.
        // Game loading time (in frames) may fluctuate, this makes replaying pre-recorded input
        // non-deterministic.
        // Automatic RNG index increment should be disabled during loading.
        EnableRngAutoIncrement(false);
    }

    void PatchMainLoop()
    {
        gg_globals globals = m_globals_orig;
        // return before directx, xaudio, steam callbacks
        globals.advance_frame_end_asm.get()[0] = 0x5F; // pop edi
        globals.advance_frame_end_asm.get()[1] = 0x5E; // pop esi
        globals.advance_frame_end_asm.get()[2] = 0x5B; // pop ebx
        globals.advance_frame_end_asm.get()[3] = 0xC3; // ret
        // replace game main loop with ggpo-compatible implementation
        globals.gg_main_loop_func.get().set(gg_main_loop);
        // replace fps function with more accurate implementation (64-bit)
        // for > 60 fps replay fast-forward
        globals.get_current_fps_func.get().set(get_current_fps);
        // don't call built-in fps limiter
        for (size_t i = 0; i < 5; ++i)
            globals.call_fps_sleep_func_asm.get()[i] = 0x90; // nop
        globals.get_input_func.get().set(get_input_from_cache);
        dump_global_data(m_image_base, globals);
    }

    // Should be called after the game has finished loading
    bool InitializeRemainingHooks()
    {
        gg_globals globals;
        load_global_data(m_image_base, globals);
        if (!globals.play_sound_func.ptr)
            return false;

        // get newly initialized global values
        m_globals_orig.play_sound_func = globals.play_sound_func;
        m_globals_orig.hwnd = globals.hwnd;
        m_globals_orig.direct3d9 = globals.direct3d9;

        // replace XAudio function with our hook
        globals.play_sound_func.ptr = play_sound;
        dump(globals.play_sound_func, m_image_base);

        return true;
    }

    void EnablePauseMenu(bool enable) final
    {
        const auto addr = (size_t)m_image_base + 0xEBC19;
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

    void SetFpsLimit(uint32_t fps) final
    {
        m_fps = fps;
    }

    uint32_t GetFpsLimit() const final
    {
        return m_fps;
    }

    uint32_t GetCurrentFps() const final
    {
        if (m_fps_timestamp_idx < 0x20)
        {
            if (m_fps_timestamp_idx < 2)
                return 60;
            auto last_timestamp = m_fps_timestamps[(m_fps_timestamp_idx - 1) & 0x1f];
            auto prev_timestamp = m_fps_timestamps[(m_fps_timestamp_idx - 2) & 0x1f];
            return static_cast<uint32_t>(
                std::round(double(1000000) / (last_timestamp - prev_timestamp))
            );
        }
        else
        {
            auto ratio = m_fps / 60;
            if (ratio == 0)
                ratio = 1;
            auto last_timestamp = m_fps_timestamps[(m_fps_timestamp_idx - 1) & 0x1f];
            auto first_timestamp = m_fps_timestamps[(m_fps_timestamp_idx) & 0x1f];
            return static_cast<uint32_t>(
                std::round(double(1000000 * (m_fps_timestamps.size() - 1) * ratio) / (last_timestamp - first_timestamp))
            );
        }
    }

    void EnableRngAutoIncrement(bool enable)
    {
        const auto addr = (size_t)m_image_base + 0x4323C;
        if (enable)
        {
            // 75 = jne
            const uint8_t data = 0x75;
            local_memory_accessor::write(data, addr);
        }
        else
        {
            // eb = jmp
            const uint8_t data = 0xeb;
            local_memory_accessor::write(data, addr);
        }
    }

    const game_state& GetState() const final
    {
        return m_state;
    }

    void SetState(const game_state& state) final
    {
        std::unordered_set<LPVOID> remaining_fibers;
        for (const auto& f : state.fibers)
            remaining_fibers.insert(f.shared->fiber);
        for (const auto& f : m_state.fibers)
        {
            const auto fiber = f.shared->fiber;
            if (remaining_fibers.find(fiber) != remaining_fibers.end())
                continue;
            // release orphaned fibers
            g_fiber_service->release(fiber);
        }

        m_state = state;
        revert_state(m_image_base, m_state, g_fiber_service.get());
    }

    void ReadInput() final
    {
        const auto f = *m_globals_orig.get_input_func.get().get();

        input_data input{};
        f(&input);

        m_input[0] = input.keys[0];
        m_input[1] = input.keys[1];

        if (m_ready)
        {
            for (const auto& func : m_callbacks[IGame::Event::AfterReadInput])
            {
                if (!func(this))
                    break;
            }
        }
    }

    void DrawFrame(bool update_fps_timestamps) final
    {
        bool skip = false;
        if (m_ready)
        {
            for (const auto& func : m_callbacks[IGame::Event::BeforeDrawFrame])
            {
                if (!func(this))
                {
                    skip = true;
                    break;
                }
            }
        }

        if (!skip)
        {
            // taken from :base+146FFB
            auto cond1 = *reinterpret_cast<uint32_t*>(m_image_base + 0x50654C);
            if (cond1 & 2)
                return;

            auto cond2 = *reinterpret_cast<uint32_t*>(m_image_base + 0x555FDC);
            if (!cond2)
            {
                auto f = *m_globals_orig.draw1_func.get();
                f(1);
            }
            auto f = *m_globals_orig.draw2_func.get();
            f();
        }

        if (update_fps_timestamps)
            UpdateFpsTimestamps();

        if (m_ready)
        {
            for (const auto& func : m_callbacks[IGame::Event::AfterDrawFrame])
            {
                if (!func(this))
                    break;
            }
        }
    }

    void UpdateFpsTimestamps()
    {
        auto ratio = m_fps / 60;
        if (!ratio || GetState().match.frame % ratio == 0)
        {
            using std::chrono::time_point_cast;
            using std::chrono::microseconds;
            using std::chrono::high_resolution_clock;
            auto mcs = time_point_cast<microseconds>(high_resolution_clock::now()).time_since_epoch().count();
            m_fps_timestamps[m_fps_timestamp_idx++ & 0x1f] = mcs;
            if (m_fps_timestamp_idx == 0x40)
                m_fps_timestamp_idx = 0x20;
        }
    }

    void ProcessAudio() final
    {
        auto func = m_globals_orig.process_audio_func.get();
        func();

        if (!m_ready && LateInitialize())
            m_ready = true;
    }

    void RunSteamCallbacks() final
    {
        auto func = m_globals_orig.run_steam_callbacks_func_ptr.get();
        func();
    }

    input_t GetCachedInput() const final
    {
        return m_input;
    }

    void SetCachedInput(const input_t& input) final
    {
        m_input = input;
    }

    input_t RemapInputToDefault(input_t input) const final
    {
        const auto& controller_configs = g_game->GetGameConfig().player_controller_config;
        input[0] = g_game->RemapButtons(
            m_input[0], controller_configs[0], game_config::default_controller_config
        );
        input[1] = g_game->RemapButtons(
            m_input[1], controller_configs[1], game_config::default_controller_config
        );
        return input;
    }

    input_t RemapInputFromDefault(input_t input) final
    {
        const auto& controller_configs = g_game->GetGameConfig().player_controller_config;
        input[0] = g_game->RemapButtons(
            input[0], game_config::default_controller_config, controller_configs[0]
        );
        input[1] = g_game->RemapButtons(
            input[1], game_config::default_controller_config, controller_configs[1]
        );
        return input;
    }

    void DisplayPlayerStatusTicker(const char* message, uint32_t side) final
    {
        auto f = m_globals_orig.player_status_ticker;
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

    void DrawRect(uint32_t color, uint32_t x1, uint32_t y1,
                          uint32_t x2, uint32_t y2) final
    {
        auto f = m_globals_orig.draw_rect;
        __asm
        {
            push 4
            push y2
            push x2
            push y1
            push x1
            mov eax, color
            call f
            add esp, 4*5
        }
    }

    void WriteUtf8Font(const char* text, int x, int y,
                               float z, float opacity,
                               float scale_x = 1, float scale_y = 1) final
    {
        float scale_x_orig, scale_y_orig;
        load(m_image_base + 0x3EE774, scale_x_orig);
        load(m_image_base + 0x3EE83C, scale_y_orig);
        dump(scale_x, m_image_base + 0x3EE774);
        dump(scale_y, m_image_base + 0x3EE83C);
        auto f = m_globals_orig.write_utf8_font;
        __asm
        {
            push 0
            push 0xFFFFFFFF
            push opacity
            push z
            push y
            push x
            mov eax, text
            call f
            add esp, 4*6
        }
        dump(scale_x_orig, m_image_base + 0x3EE774);
        dump(scale_y_orig, m_image_base + 0x3EE83C);
    }

    void WriteCockpitFont(const char* buffer, int x, int y, float z,
                                  uint8_t alpha, float scale) final
    {
        m_globals_orig.write_cockpit_font(buffer, x, y, z, alpha, scale);
    }

    void WriteSpecialFont(const char* text, float x, float y, float z,
                                  uint32_t flags, uint32_t font, float scale) final
    {
        m_globals_orig.write_special_font(text, x, y, z, flags, font, scale);
    }

    void DrawPressedButtons(uint32_t input_bitmask, const gg_char_state& player_state, uint32_t x, uint32_t y) final
    {
        // TODO: reimplement, this doesn't work in menus
        const auto buttons = button_bitmask_to_icon_bitmask(input_bitmask, player_state);
        m_globals_orig.draw_pressed_buttons(buttons, x, y, 3, 1);
    }

    void DrawPressedDirection(uint32_t input_bitmask, uint32_t x, uint32_t y) final
    {
        const auto directions = direction_bitmask_to_icon_id(input_bitmask);
        m_globals_orig.draw_pressed_buttons(directions, x, y, 3, 1);
    }

    uint16_t RemapButtons(uint16_t input, const game_config::controller_config& from,
                          const game_config::controller_config& to) const final
    {
        input = reverse_bytes(input);

        const auto directions = from.up.bit | from.down.bit |
                                from.left.bit | from.right.bit;
        uint16_t result = input & directions;

        if (input & from.pk.bit)
            input |= to.p.bit | to.k.bit;
        if (input & from.pd.bit)
            input |= to.p.bit | to.d.bit;
        if (input & from.pks.bit)
            input |= to.p.bit | to.k.bit | to.s.bit;
        if (input & from.pksh.bit)
            input |= to.p.bit | to.k.bit | to.s.bit | to.hs.bit;

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

        if (input & from.enemy_jump.bit)
            result |= to.enemy_jump.bit;
        if (input & from.enemy_walk.bit)
            result |= to.enemy_walk.bit;
        if (input & from.play_memory.bit)
            result |= to.play_memory.bit;
        if (input & from.rec_enemy.bit)
            result |= to.rec_enemy.bit;
        if (input & from.rec_player.bit)
            result |= to.rec_player.bit;
        if (input & from.switch_control.bit)
            result |= to.switch_control.bit;

        return reverse_bytes(result);
    }

    const game_config& GetGameConfig() const final
    {
        return m_state.match.config.get();
    }

    bool InMatch() const final
    {
        const auto& fibers = m_state.match.menu_fibers.get();
        if (m_state.match.next_fiber_id != fiber_id::match)
            return false;
        for (const auto& f : fibers)
        {
            if (f.name == std::string_view("FIN "))
                continue;
            if (f.name == std::string_view("NXBT"))
                continue;
            if (f.name == std::string_view("BFBT"))
                continue;
            if (f.status)
                return false;
        }
        return true;
    }

    bool InTrainingMode() const final
    {
        const auto game_mode = m_state.match.game_mode.get();
        return game_mode & 0x100;
    }

    bool InVs2p() const final
    {
        const auto game_mode = m_state.match.game_mode.get();
        return game_mode & 0x800;
    }

    uint32_t GetActivePlayers() const final
    {
        const auto game_mode = m_state.match.game_mode.get();
        return game_mode & 0x3;
    }

    bool FindFiberByName(const std::string_view& name) const final
    {
        const auto& fibers = m_state.match.menu_fibers.get();
        for (const auto& f : fibers)
        {
            if (f.name == name)
                return true;
        }
        return false;
    }

    bool LateInitialize()
    {
        decltype(match_state::menu_fibers) fibers;
        load(m_image_base, fibers);
        const auto& fiber_data = fibers.get()[0];
        // wait until the game has loaded (ie Loading fiber has exited)
        const auto is_loading = fiber_data.name == std::string_view("ATLD");
        if (!is_loading && InitializeRemainingHooks())
        {
            if (!g_fiber_service)
                g_fiber_service = fiber_mgmt::fiber_service::start();

            EnableRngAutoIncrement(true);

            save_current_state(m_image_base, m_state);

            // Reset game clock to zero, we'll be using it as frame counter
            // Also, same game state with different clock values may produce
            // different results (IK animation timings, wake-up timings?)
            m_state.match.frame = 0;
            dump_unprotected(m_state.match.frame, m_image_base);

            return true;
        }
        return false;
    }

    void AdvanceFrame() final
    {
        if (m_ready)
        {
            for (const auto& func : m_callbacks[IGame::Event::BeforeAdvanceFrame])
            {
                if (!func(this))
                    return;
            }
        }

        const auto f = *m_globals_orig.gg_main_loop_func.get();
        f();

        if (m_ready)
        {
            // before reading game state, wait for background workers
            m_globals_orig.wait_file_readers();
            m_globals_orig.xaudio_read_pending_files(*reinterpret_cast<void**>(m_image_base + 0x556020));

            save_current_state(m_image_base, m_state, g_fiber_service.get());
            for (const auto& func : m_callbacks[IGame::Event::AfterAdvanceFrame])
            {
                if (!func(this))
                    break;
            }
        }
    }

    int32_t MsTillNextFrame() const final
    {
        auto now = std::chrono::high_resolution_clock::now();
        auto ms_left = std::chrono::duration_cast<std::chrono::milliseconds>(m_next_frame - now).count();
        return static_cast<int32_t>(ms_left);
    }

    bool Idle() final
    {
        m_frame_aborted = false;

        if (m_ready)
        {
            for (auto func : m_callbacks[IGame::Event::Idle])
            {
                if (!func(this))
                    return true;
            }
        }

        const auto now = std::chrono::high_resolution_clock::now();
        if (m_fps)
        {
            bool still_idle = now + std::chrono::microseconds(500) < m_next_frame;
            if (!still_idle)
            {
                std::chrono::nanoseconds increment = std::chrono::seconds(1);
                increment /= m_fps;
                auto fps = GetCurrentFps();
                uint32_t delta = std::max(m_fps / 60, 1u);
                if (fps + delta < m_fps)
                    m_next_frame = now + increment;
                else
                    m_next_frame += increment;
            }
            return still_idle;
        }
        else
        {
            m_next_frame = now;
            return false;
        }
    }

    void RestartIfRequested() final
    {
        // taken from :base+14701C
        int flag = *reinterpret_cast<int*>(m_image_base + 0x555BB4);
        if (flag == 0)
            return;
        m_globals_orig.restart_process_func();
    }

    void AbortCurrentFrame() final
    {
        m_frame_aborted = true;
    }

    bool IsCurrentFrameAborted() const final
    {
        return m_frame_aborted;
    }

    const std::pair<IXACT3WaveBank*, int16_t>& GetCurrentSound() const final
    {
        return m_sound;
    }

    size_t GetImageBase() const final
    {
        return m_image_base;
    }

    HWND GetWindowHandle() const final
    {
        return m_globals_orig.hwnd;
    }

    IDirect3DDevice9* GetDirect3D9Device() const final
    {
        return *m_globals_orig.direct3d9;
    }

    void RegisterCallback(Event event, CallbackFuncType f, CallbackPosition pos) final
    {
        if (pos == CallbackPosition::Last)
            m_callbacks[event].push_back(f);
        else
            m_callbacks[event].insert(m_callbacks[event].begin(), f);
    }

    bool CheckDeviceLost()
    {
        // flag at 0x504F3C is set if Present() fails
        bool& device_lost = *reinterpret_cast<bool*>(m_image_base + 0x504F3C);
        if (device_lost)
        {
            if (!m_globals_orig.reset_directx_device())
                device_lost = false;
        }
        return device_lost;
    }

private:
    friend int32_t play_sound_impl(IXACT3WaveBank*, int16_t, uint32_t, int32_t, int8_t, IXACT3Wave**);
    friend void gg_main_loop_impl();

    gg_globals m_globals_orig;
    std::chrono::high_resolution_clock::time_point m_next_frame = std::chrono::high_resolution_clock::now();
    uint32_t m_fps = 60;
    std::unordered_map<IGame::Event, std::vector<IGame::CallbackFuncType>> m_callbacks;
    game_state m_state{};
    std::pair<IXACT3WaveBank*, int16_t> m_sound = { nullptr, static_cast<int16_t>(0) };
    size_t m_image_base = 0;
    bool m_ready = false;
    input_t m_input;
    bool m_frame_aborted = false;
    size_t m_fps_timestamp_idx = 0;
    std::array<uint64_t, 0x20> m_fps_timestamps = { 0 };

    uint32_t button_bitmask_to_icon_bitmask(uint32_t input, const gg_char_state& state)
    {
        gg_object state_wrapper;
        state_wrapper.char_state_ptr = &state;
        const auto obj = &state_wrapper;
        auto f = m_globals_orig.button_bitmask_to_icon_bitmask;
        __asm
        {
            mov ecx, input
            mov edx, obj
            push 1
            call f
            add esp, 4
        }
    }

    uint32_t direction_bitmask_to_icon_id(uint32_t input)
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

        auto f = m_globals_orig.direction_bitmask_to_icon_id;
        __asm
        {
            mov ecx, input_
            push 0
            push input_
            call f
            add esp, 4*2
        }
    }
};

int32_t play_sound_impl(IXACT3WaveBank* a1, int16_t a2, uint32_t a3, int32_t a4, int8_t a5, IXACT3Wave** a6)
{
    auto game = dynamic_cast<Game*>(g_game);
    if (game->m_ready)
    {
        game->m_sound = { a1, a2 };
        for (const auto& func : game->m_callbacks[IGame::Event::BeforePlaySound])
        {
            if (!func(g_game))
            {
                *a6 = &g_empty_wave;
                game->m_sound = { nullptr, static_cast<int16_t>(0) };
                return 0;
            }
        }
    }

    const auto f = *game->m_globals_orig.play_sound_func.ptr;
    return f(a1, a2, a3, a4, a5, a6);
}

void gg_main_loop_impl()
{
    auto game = dynamic_cast<Game*>(g_game);
    if (game->CheckDeviceLost())
        return;
    if (!g_game->Idle())
    {
        g_game->ReadInput();
        if (!g_game->IsCurrentFrameAborted())
        {
            g_game->AdvanceFrame();
            g_game->DrawFrame();
            g_game->ProcessAudio();
            g_game->RunSteamCallbacks();
            g_game->RestartIfRequested();
        }
    }
}

std::shared_ptr<IGame> IGame::Initialize(size_t baseAddress, configuration* cfg)
{
    assert(g_game == nullptr);
    auto game = std::make_shared<Game>(baseAddress, cfg);
    g_game = game.get();
    return game;
}
