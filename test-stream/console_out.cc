#include "console_out.h"

#include <cstdio>

ConsoleOut::ConsoleOut() {}

void ConsoleOut::flush() {
    for(int i = 0; i < pos; i++) putchar(buffer[i]);
    pos = 0;
}
