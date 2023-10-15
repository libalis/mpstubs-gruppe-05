#include "outputstream.h"

OutputStream& OutputStream::operator << (char c) {
    put(c);
    return *this;
}

OutputStream& OutputStream::operator << (unsigned char c) {
    char p = static_cast<char>(c);
	put(p);
	return *this;
}

OutputStream& OutputStream::operator << (const char* string) {
    char c;
    int i = 0;
    while((c = string[i++]) != '\0') put(c);
    return *this;
}

OutputStream& OutputStream::operator << (bool b) {
    b ? (*this << "true") : (*this << "false");
    return *this;
}

OutputStream& OutputStream::helper(unsigned long long ival, bool sign = false) {
    if (base == 2) *this << "0b";
    else if (base == 8) *this << '0';
    else if (base == 16) *this << "0x";
    if (ival == 0) {
        return (*this << '0');
    }
    char binary[2] = {'0', '1'};
    char octal[8] = {'0', '1', '2', '3', '4', '5', '6', '7'};
    char decimal[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    char *all[17];
    all[2] = binary;
    all[8] = octal;
    all[10]= decimal;
    all[16] = hex;
    int size = 0;
    for(unsigned long long ival_copy = ival; ival_copy != 0; ival_copy /= base) size++;
    char *tmp = new char[size];
    for (int i = 0; ival != 0; ival /= base) tmp[i++] = all[base][ival%base];
    if (sign) {
            char *out = new char[size+2];
            out[0] = '-';
            for (int i = 0; i < size; i++) out[i + 1] = tmp[size-i-1];
            out[size+1] = '\0';
            return (*this << out);
    } else {
            char *out = new char[size+1];
            for (int i = 0; i < size; i++) out[i] = tmp[size-i-1];
            out[size] = '\0';
            return (*this << out);
    }
}

OutputStream& OutputStream::operator << (short ival) {
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) : helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned short ival) {
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (int ival) {
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) : helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned int ival){
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (long ival){
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) : helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned long ival){
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (long long ival){
    return (ival < 0) ? helper(static_cast<unsigned long long>(ival * -1), true) : helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (unsigned long long ival){
    return helper(static_cast<unsigned long long>(ival));
}

OutputStream& OutputStream::operator << (const void* ptr) {
    int base_copy = base;
    base = 16;
    helper(reinterpret_cast<unsigned long long>(ptr));
    base = base_copy;
    return *this;
}

OutputStream& OutputStream::operator << (OutputStream& (*f) (OutputStream&)){
    return f(*this);
}


OutputStream& flush(OutputStream& os){
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
