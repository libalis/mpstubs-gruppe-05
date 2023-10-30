#include "machine/textmode.h"
#include "machine/ioport.h"

void TextMode::setCursor(unsigned abs_x, unsigned abs_y) {
    IOPort index_register = IOPort(0x3d4);
    IOPort daten_register = IOPort(0x3d5);
    uint16_t position = abs_x + abs_y * COLUMNS;
    uint8_t position_high = position >> 8;
    uint8_t position_low = position & 0xff;
    index_register.outb(14);
    daten_register.outb(position_high);
    index_register.outb(15);
    daten_register.outb(position_low);
}

void TextMode::getCursor(unsigned& abs_x, unsigned& abs_y) {
    IOPort index_register = IOPort(0x3d4);
    IOPort daten_register = IOPort(0x3d5);
    index_register.outb(14);
    uint8_t position_high = daten_register.inb();
    index_register.outb(15);
    uint8_t position_low = daten_register.inb();
    uint16_t position = (position_high << 8) + position_low;
    abs_x = position % COLUMNS;
    abs_y = position / COLUMNS;
}

void TextMode::show(unsigned abs_x, unsigned abs_y, char character, Attribute attrib) {
    uint16_t position = abs_x + abs_y * COLUMNS;
    char* address_character = reinterpret_cast<char*>(0xb8000 + position * 2);
    *address_character = character;
    Attribute* address_attrib = reinterpret_cast<Attribute*>(0xb8000 + position * 2 + 1);
    *address_attrib = attrib;
}
