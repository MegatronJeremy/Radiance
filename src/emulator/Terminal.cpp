#include "../../inc/emulator/Terminal.hpp"

#include <unistd.h>
#include <fcntl.h>

// RAII object
Terminal::Terminal() {
    disableInputBuffering();
}

Terminal::~Terminal() {
    restoreInputBuffer();
}

void Terminal::disableInputBuffering() {
    // get original settings
    tcgetattr(STDIN_FILENO, &original_settings);

    // disable canonical mode and echo
    new_settings = original_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    // make input non-blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void Terminal::restoreInputBuffer() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
}



