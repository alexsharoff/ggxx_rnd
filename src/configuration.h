#pragma once

#include "libgg_args.h"
#include "libgg_settings.h"


class configuration
{
public:
    configuration();

    const libgg_args& get_args() const;
    const skip_intro_settings& get_skip_intro_settings() const;
    const recorder_settings& get_recorder_settings() const;
    void set_skip_intro_settings(const skip_intro_settings& settings);

private:
    libgg_args m_args;

    struct libgg_settings
    {
        skip_intro_settings skip_intro;
        recorder_settings recorder;
    } m_settings;
};
