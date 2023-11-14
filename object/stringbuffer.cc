#include "object/stringbuffer.h"

void Stringbuffer::put(char c) {
    buffer[pos++] = c;
    if (pos == sizeof(buffer)/sizeof(buffer[0])) {
        flush();
        pos = 0;
    }
}
