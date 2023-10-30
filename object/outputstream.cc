#include "object/outputstream.h"

OutputStream& OutputStream::operator << (char c) {
    put(c);
    return *this;
}

OutputStream& OutputStream::operator << (unsigned char c) {
    return (*this << static_cast<char>(c));
}

OutputStream& OutputStream::operator << (const char* string) {
    for (int i = 0; string[i] != '\0'; i++) (*this << string[i]);
    return *this;
}

OutputStream& OutputStream::operator << (bool b) {
    b ? (*this << "true") : (*this << "false");
    return *this;
}

OutputStream& OutputStream::helper(unsigned long long ival, bool sign = false) {
    if (ival == 0) return *this << '0';
    if (base == 2) *this << "0b";
    else if (base == 8) *this << "0";
    else if (base == 10 && sign) *this << '-';
    else if (base == 16) *this << "0x";
    char hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    auto helper = [&](auto ival, auto&& helper) -> void {
        if (ival / base > 0) helper(ival / base, helper);
        *this << hex_chars[ival % base];
    };
    helper(ival, helper);
    return *this;
}

OutputStream& OutputStream::operator << (short ival) {
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) :
        helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned short ival) {
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (int ival) {
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) :
        helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned int ival) {
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (long ival) {
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) :
        helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned long ival) {
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (long long ival) {
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) :
        helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned long long ival) {
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (const void* ptr) {
    int base_copy = base;
    base = 16;
    helper(reinterpret_cast<unsigned long long>(ptr));
    base = base_copy;
    return *this;
}

OutputStream& OutputStream::operator << (OutputStream& (*f) (OutputStream&)) {
    return f(*this);
}


OutputStream& flush(OutputStream& os) {
    os.flush();
    return os;
}

OutputStream& endl(OutputStream& os) {
    os << '\n';
    os.flush();
    return os;
}

OutputStream& bin(OutputStream& os) {
    os.base = 2;
    return os;
}

OutputStream& oct(OutputStream& os) {
    os.base = 8;
    return os;
}

OutputStream& dec(OutputStream& os) {
    os.base = 10;
    return os;
}

OutputStream& hex(OutputStream& os) {
    os.base = 16;
    return os;
}
