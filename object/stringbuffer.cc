#include "stringbuffer.h"

void Stringbuffer::put(char c) {
    buffer[pos++] = c;
    if (pos == 80) {
        flush();
        pos = 0;
    }
}
