#include "machine/textwindow.h"

TextWindow::TextWindow(unsigned from_col, unsigned to_col, unsigned from_row, unsigned to_row, bool use_cursor) :
    from_col(from_col), to_col(to_col), from_row(from_row), to_row(to_row), use_cursor(use_cursor) {
        if (use_cursor) return;
        pos_x = from_col;
        pos_y = from_row;
    }

void TextWindow::setPos(unsigned rel_x, unsigned rel_y) {
    if (rel_x < from_col || rel_x >= to_col || rel_y < from_row || rel_y >= to_row) return;
    if (use_cursor) {
        TextMode::setCursor(rel_x + from_col, rel_y + from_row);
        return;
    }
    pos_x = rel_x + from_col;
    pos_y = rel_y + from_row;
}

void TextWindow::getPos(unsigned& rel_x, unsigned& rel_y) const {
    if (use_cursor) {
        TextMode::getCursor(rel_x, rel_y);
        rel_x -= from_col;
        rel_y -= from_row;
        return;
    }
    rel_x = pos_x - from_col;
    rel_y = pos_y - from_row;
}

void TextWindow::print(const char* string, size_t length, Attribute attrib) {
    auto helper = [](unsigned from_row, unsigned to_row, unsigned from_col, unsigned to_col) {
        for (unsigned int y = from_row; y < to_row - 1; y++) {
            for (unsigned int x = from_col; x < to_col; x++) {
                uint16_t position = x + (y + 1) * COLUMNS;
                char* address_character = reinterpret_cast<char *>(0xb8000 + position * 2);
                Attribute* address_attrib = reinterpret_cast<Attribute*>(0xb8000 + position * 2 + 1);
                TextMode::show(x, y, *address_character, *address_attrib);
            }
        }
        for (unsigned int x = from_col; x < to_col; x++) TextMode::show(x, to_row - 1, ' ');
    };
    for (size_t i = 0; i < length; i++) {
        unsigned abs_x;
        unsigned abs_y;
        getPos(abs_x, abs_y);
        abs_x += from_col;
        abs_y += from_row;
        if (string[i] == '\n') {
            abs_x = from_col;
            abs_y++;
            if (abs_y >= to_row) {
                abs_y = to_row - 1;
                helper(from_row, to_row, from_col, to_col);
            }
            setPos(abs_x - from_col, abs_y - from_row);
            continue;
        }
        TextMode::show(abs_x, abs_y, string[i], attrib);
        abs_x++;
        if (abs_x >= to_col) {
            abs_x = from_col;
            abs_y++;
            if (abs_y >= to_row) {
                abs_y = to_row - 1;
                helper(from_row, to_row, from_col, to_col);
                }
        }
        setPos(abs_x - from_col, abs_y - from_row);
    }
}

void TextWindow::reset(char character, Attribute attrib) {
    for (unsigned int y = from_row; y < to_row; y++) {
        for (unsigned int x = from_col; x < to_col; x++) {
            TextMode::show(x, y, character, attrib);
        }
    }
    setPos(0, 0);
}
