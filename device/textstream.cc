#include "device/textstream.h"

void TextStream::flush() {
    TextWindow::print(Stringbuffer::buffer, Stringbuffer::pos);
    Stringbuffer::pos = 0;
}
