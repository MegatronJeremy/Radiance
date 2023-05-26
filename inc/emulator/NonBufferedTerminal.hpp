#pragma once

#include <termios.h>

class NonBufferedTerminal {
public:
    NonBufferedTerminal();

    ~NonBufferedTerminal();

private:
    void disableInputBuffering();

    void restoreInputBuffer();

    termios new_settings{};
    termios original_settings{};
};