#pragma once

#include <termios.h>

class Terminal {
public:
    Terminal();

    ~Terminal();

private:
    void disableInputBuffering();

    void restoreInputBuffer();

    termios new_settings{};
    termios original_settings{};
};