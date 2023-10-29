#include "user/app1/appl.h"
#include "object/outputstream.h"
#include "types.h"

/*
TextStream kout = {0, COLUMNS, 0, ROWS-Core::count()/2*3, on};
for (unsigned i = 0; i < Core::count(); i++) {
    TextStream dout[i] = {i%2*TextMode::COLUMNS/2, (i%2+1)*TextMode::COLUMNS/2, TextMode::ROWS-Core::count()/2*3+i/2*3, TextMode::ROWS-Core::count()/2*3+i/2*3+3};
}
*/

void Application::action() {
    kout << "Test          <stream result> -> <expected>" << endl;
    kout << "bool:         " << true << " -> true" << endl;
    kout << "zero:         " << 0 << " -> 0" << endl;
    kout << "binary:       " << bin << 42 << dec << " -> 0b101010" << endl;
    kout << "octal:        " << oct << 42 << dec << " -> 052" << endl;
    kout << "hex:          " << hex << 42 << dec << " -> 0x2a" << endl;
    kout << "uint64_t max: " << ~((uint64_t)0) << " -> 18446744073709551615" << endl;
    kout << "int64_t max:  " << ~(1ll<<63) << " -> 9223372036854775807" << endl;
    kout << "int64_t min:  " << (1ll<<63) << " -> -9223372036854775808" << endl;
    kout << "some int64_t: " << (-1234567890123456789) << " -> -1234567890123456789" << endl;
    kout << "some int64_t: " << (1234567890123456789) << " -> 1234567890123456789" << endl;
    kout << "pointer:      " << reinterpret_cast<void*>(1994473406541717165ull) << " -> 0x1badcafefee1dead" << endl;
    kout << "smiley:       " << static_cast<char>(1) << endl;
}
