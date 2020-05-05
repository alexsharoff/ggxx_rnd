#pragma once

#include "libgg_args.h"
#include "libgg_settings.h"


class configuration
{
public:
    configuration();
    ~configuration();

    const libgg_args& get_args() const;
    const skip_intro_settings& get_skip_intro_settings() const;
    const keyboard_mapping& get_keyboard_mapping() const;
    void set_skip_intro_settings(const skip_intro_settings& settings);

private:
    libgg_args m_args;

    struct libgg_settings
    {
        skip_intro_settings skip_intro;
        keyboard_mapping keymap;
    } m_settings;
};
