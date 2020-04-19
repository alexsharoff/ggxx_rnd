#pragma once

#include "configuration.h"
#include "game_state.h"

#include <array>
#include <functional>
#include <memory>


struct IGame
{
    using CallbackFuncType = std::function<bool(IGame*)>;
    enum class Event : uint8_t
    {
        BeforeGameTick,
        AfterGameTick,
        BeforeSleep,
        BeforePlaySound,
        AfterGetInput,
        AfterProcessObjects
    };
    enum class CallbackPosition : uint8_t
    {
        First,
        Last
    };
    using input_t = std::array<uint16_t, 2>;

    virtual void EnableDrawing(bool enable) = 0;
    virtual void EnablePauseMenu(bool enable) = 0;
    virtual void EnableFpsLimit(bool enable) = 0;
    // Each frame, RNG (Mersenne Twister) index is incremented even if it was unused.
    // Game loading time (in frames) may fluctuate, this makes replaying pre-recorded input
    // non-deterministic.
    // Automatic RNG index increment should be disabled during loading.
    virtual void AutoIncrementRng(bool enable) const = 0;

    virtual const game_state& GetState() const = 0;
    virtual const input_t& GetInput() const = 0;
    virtual const input_t GetInputRemapped() const = 0;

    virtual void SetState(const game_state& state) = 0;
    virtual void SetInput(const input_t& input) = 0;
    virtual void SetInputRemapped(const input_t& input) = 0;

    virtual void DisplayPlayerStatusTicker(const char* message, uint32_t side) = 0;
    virtual void DrawRect(uint32_t color, uint32_t x1, uint32_t y1,
                          uint32_t x2, uint32_t y2) = 0;
    virtual void WriteUtf8Font(const char* text, int x, int y,
                               float z, float opacity,
                               float scale_x = 1, float scale_y = 1) = 0;
    virtual void WriteCockpitFont(const char* buffer, int x, int y, float z,
                                  uint8_t alpha, float scale) = 0;
    virtual void WriteSpecialFont(const char* text, float x, float y, float z,
                                  uint32_t flags, uint32_t font, float scale) = 0;
    virtual void DrawPressedButtons(uint32_t input_bitmask, const gg_char_state& player_state, uint32_t x, uint32_t y) = 0;
    virtual void DrawPressedDirection(uint32_t input_bitmask, uint32_t x, uint32_t y) = 0;
    virtual uint16_t RemapButtons(uint16_t input, const game_config::controller_config& from,
                                  const game_config::controller_config& to) const = 0;
    virtual const game_config& GetGameConfig() const = 0;
    virtual bool InMatch() const = 0;
    virtual bool InTrainingMode() const = 0;
    virtual bool InVs2p() const = 0;
    virtual uint32_t GetActivePlayers() const = 0;
    virtual bool FindFiberByName(const std::string& name) const = 0;
    virtual const std::pair<IXACT3WaveBank*, int16_t>& GetCurrentSound() const = 0;
    virtual const uint32_t GetSleepTime() const = 0;

    virtual void GameTick() const = 0;
    virtual void GetInputRaw(input_data* out) const = 0;
    virtual void ProcessInput() const = 0;
    virtual void ProcessObjects() const = 0;
    virtual int32_t LimitFps() const = 0;

    virtual size_t GetImageBase() const = 0;
    virtual HWND GetWindowHandle() const = 0;

    virtual void RegisterCallback(Event event, CallbackFuncType f, CallbackPosition pos = CallbackPosition::Last) = 0;

    static std::shared_ptr<IGame> Initialize(size_t baseAddress, configuration* cfg);
};
