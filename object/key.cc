#include "object/key.h"

// Character table for scan codes for US keyboards
static struct {
	const unsigned char normal,  // Character without modifiers
	                    shift,   // Character with pressed Shift, Capslock, or in Numpad
	                    alt;     // Character with pressed Alt key
} ascii_tab[Key::Scancode::KEYS] = {
	{   0,   0,   0 },  // KEY_INVALID
	{   0,   0,   0 },  // KEY_ESCAPE
	{ '1', '!',   0 },  // KEY_1
	{ '2', '"', 253 },	// KEY_2
	{ '3',  21,   0 },  // KEY_3
	{ '4', '$',   0 },  // KEY_4
	{ '5', '%',   0 },  // KEY_5
	{ '6', '&',   0 },  // KEY_6
	{ '7', '/', '{' },  // KEY_7
	{ '8', '(', '[' },  // KEY_8
	{ '9', ')', ']' },  // KEY_9
	{ '0', '=', '}' },  // KEY_0
	{ 225, '?', '\\'},  // KEY_DASH
	{  39,  96,   0 },  // KEY_EQUAL
	{'\b',   0,   0 },  // KEY_BACKSPACE
	{   0,   0,   0 },  // KEY_TAB
	{ 'q', 'Q', '@' },  // KEY_Q
	{ 'w', 'W',   0 },  // KEY_W
	{ 'e', 'E',   0 },  // KEY_E
	{ 'r', 'R',   0 },  // KEY_R
	{ 't', 'T',   0 },  // KEY_T
	{ 'z', 'Z',   0 },  // KEY_Y
	{ 'u', 'U',   0 },  // KEY_U
	{ 'i', 'I',   0 },  // KEY_I
	{ 'o', 'O',   0 },  // KEY_O
	{ 'p', 'P',   0 },  // KEY_P
	{ 129, 154,   0 },  // KEY_OPEN_BRACKET
	{ '+', '*', '~' },  // KEY_CLOSE_BRACKET
	{'\n',   0,   0 },  // KEY_ENTER
	{   0,   0,   0 },  // KEY_LEFT_CTRL
	{ 'a', 'A',   0 },  // KEY_A
	{ 's', 'S',   0 },  // KEY_S
	{ 'd', 'D',   0 },  // KEY_D
	{ 'f', 'F',   0 },  // KEY_F
	{ 'g', 'G',   0 },  // KEY_G
	{ 'h', 'H',   0 },  // KEY_H
	{ 'j', 'J',   0 },  // KEY_J
	{ 'k', 'K',   0 },  // KEY_K
	{ 'l', 'L',   0 },  // KEY_L
	{ 148, 153,   0 },  // KEY_SEMICOLON
	{ 132, 142,   0 },  // KEY_APOSTROPH
	{ '^', 248,   0 },  // KEY_GRAVE_ACCENT
	{   0,   0,   0 },  // KEY_LEFT_SHIFT
	{ '#',  39,   0 },  // KEY_BACKSLASH
	{ 'y', 'Y',   0 },  // KEY_Z
	{ 'x', 'X',   0 },  // KEY_X
	{ 'c', 'C',   0 },  // KEY_C
	{ 'v', 'V',   0 },  // KEY_V
	{ 'b', 'B',   0 },  // KEY_B
	{ 'n', 'N',   0 },  // KEY_N
	{ 'm', 'M', 230 },  // KEY_M
	{ ',', ';',   0 },  // KEY_COMMA
	{ '.', ':',   0 },  // KEY_PERIOD
	{ '-', '_',   0 },  // KEY_SLASH
	{   0,   0,   0 },  // KEY_RIGHT_SHIFT
	{ '*', '*',   0 },  // KEY_KP_STAR
	{   0,   0,   0 },  // KEY_LEFT_ALT
	{ ' ', ' ',   0 },  // KEY_SPACEBAR
	{   0,   0,   0 },  // KEY_CAPS_LOCK
	{   0,   0,   0 },  // KEY_F1
	{   0,   0,   0 },  // KEY_F2
	{   0,   0,   0 },  // KEY_F3
	{   0,   0,   0 },  // KEY_F4
	{   0,   0,   0 },  // KEY_F5
	{   0,   0,   0 },  // KEY_F6
	{   0,   0,   0 },  // KEY_F7
	{   0,   0,   0 },  // KEY_F8
	{   0,   0,   0 },  // KEY_F9
	{   0,   0,   0 },  // KEY_F10
	{   0,   0,   0 },  // KEY_NUM_LOCK
	{   0,   0,   0 },  // KEY_SCROLL_LOCK
	{   0, '7',   0 },  // KEY_KP_7
	{   0, '8',   0 },  // KEY_KP_8
	{   0, '9',   0 },  // KEY_KP_9
	{ '-', '-',   0 },  // KEY_KP_DASH
	{   0, '4',   0 },  // KEY_KP_4
	{   0, '5',   0 },  // KEY_KP_5
	{   0, '6',   0 },  // KEY_KP_6
	{ '+', '+',   0 },  // KEY_KP_PLUS
	{   0, '1',   0 },  // KEY_KP_1
	{   0, '2',   0 },  // KEY_KP_2
	{   0, '3',   0 },  // KEY_KP_3
	{   0, '0',   0 },  // KEY_KP_0
	{ 127, ',',   0 },  // KEY_KP_PERIOD
	{   0,   0,   0 },  // KEY_SYSREQ
	{   0,   0,   0 },  // KEY_EUROPE_2
	{ '<', '>', '|' },  // KEY_F11
	{   0,   0,   0 },  // KEY_F12
	{   0,   0,   0 },  // KEY_KP_EQUAL
};

unsigned char Key::ascii() const {
	// Select the correct table depending on the modifier bits.
	// For the sake of simplicity, Shift and NumLock have precedence over Alt.
	// The Ctrl modifier does not have a distinct table.

	if (!valid()) {
		return '\0';
	} else if (shift
	          || (caps_lock
	               && (
	                       (scancode >= KEY_Q && scancode <= KEY_P)
	                    || (scancode >= KEY_A && scancode <= KEY_L)
	                    || (scancode >= KEY_Z && scancode <= KEY_M)
	                  )
	              )
	          || (num_lock && scancode >= KEY_KP_7 && scancode <= KEY_KP_PERIOD)
	        ) {
		return ascii_tab[scancode].shift;
	} else if (alt()) {
		return ascii_tab[scancode].alt;
	} else {
		return ascii_tab[scancode].normal;
	}
}
