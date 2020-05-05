#include "configuration.h"

#include "command_line.h"


configuration::configuration()
{
    m_args = parse_command_line();
    m_settings.keymap = keyboard_mapping{};
    m_settings.skip_intro = skip_intro_settings{};

    // TODO: load file
}

configuration::~configuration()
{
    // TODO: save file
}

const libgg_args& configuration::get_args() const
{
    return m_args;
}

const skip_intro_settings& configuration::get_skip_intro_settings() const
{
    return m_settings.skip_intro;
}

const keyboard_mapping& configuration::get_keyboard_mapping() const
{
    return m_settings.keymap;
}

manual_frame_advance_settings& configuration::get_manual_frame_advance_settings()
{
    return m_settings.frame_advance;
}

void  configuration::set_skip_intro_settings(const skip_intro_settings& settings)
{
    m_settings.skip_intro = settings;
}
