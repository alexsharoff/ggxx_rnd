#include "configuration.h"

#include "command_line.h"


configuration::configuration()
{
    m_args = parse_command_line();
    m_settings.recorder = keyboard_mapping{};

    m_settings.skip_intro = skip_intro_settings{};

    if (m_args.game_mode != libgg_args::game_mode_t::default)
    {
        m_settings.skip_intro.enter_menu = true;
        m_settings.skip_intro.enabled = true;
        if (m_args.game_mode == libgg_args::game_mode_t::network)
            m_settings.skip_intro.menu_idx = 3;
        else if (m_args.game_mode == libgg_args::game_mode_t::training)
            m_settings.skip_intro.menu_idx = 7;
        else if (m_args.game_mode == libgg_args::game_mode_t::vs2p)
            m_settings.skip_intro.menu_idx = 2;
    }
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
    return m_settings.recorder;
}

void  configuration::set_skip_intro_settings(const skip_intro_settings& settings)
{
    m_settings.skip_intro = settings;
}
