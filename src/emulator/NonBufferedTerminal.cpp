#include "../../inc/emulator/NonBufferedTerminal.hpp"

#include <unistd.h>
#include <fcntl.h>

// RAII object
NonBufferedTerminal::NonBufferedTerminal() {
    disableInputBuffering();
}

NonBufferedTerminal::~NonBufferedTerminal() {
    restoreInputBuffer();
}

void NonBufferedTerminal::disableInputBuffering() {
    // get original settings
    tcgetattr(STDIN_FILENO, &original_settings);

    // disable canonical mode and echo
    new_settings = original_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    // make input non-blocking
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void NonBufferedTerminal::restoreInputBuffer() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_settings);
}



