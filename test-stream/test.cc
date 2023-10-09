#include "console_out.h"
#include "file_out.h"

ConsoleOut cout;
FileOut foo("foo.txt");

int main() {
	cout << "Console Test  <stream result> -> <expected>" << endl;
	cout << "	bool: " << true << " -> true" << endl;
	cout << "	zero: " << 0 << " -> 0" << endl;
	cout << "	binary: " << bin << 42 << dec << " -> 0b101010" << endl;
	cout << "	octal: " << oct << 42 << dec << " -> 052" << endl;
	cout << "	hex: " << hex << 42 << dec << " -> 0x2a" << endl;
	cout << "	uint64_t max: " << ~((unsigned long long)0) << " -> 18446744073709551615" << endl;
	cout << "	int64_t max: " << ~(1ll<<63) << " -> 9223372036854775807" << endl;
	cout << "	int64_t min: " << (1ll<<63) << " -> -9223372036854775808" << endl;
	cout << "	some int64_t: " << (-1234567890123456789) << " -> -1234567890123456789" << endl;
	cout << "	some int64_t: " << (1234567890123456789) << " -> 1234567890123456789" << endl;
	cout << "	pointer: " << reinterpret_cast<void*>(1994473406541717165ull) << " -> 0x1badcafefee1dead" << endl;
	cout << endl;

	cout << "File Test" << endl
	     << "	currently open: " << FileOut::count() << endl
	     << "	writing into '" << foo.getPath() << "'..." << endl;
	foo << "C makes it easy to shoot yourself in the foot;" << endl;
	foo << "C++ makes it harder, but when you do it blows your whole leg off." << endl;
	 {
		FileOut bar("bar.txt");
		cout << "	opened the " << FileOut::count() << ". file, " << endl;
		cout << "	writing into '" << bar.getPath() << "'..." << endl;
		bar << "Anyone who claims to have the perfect programming language is either a fool or a salesman or both" << endl;
	 }
	cout << "	having only " << FileOut::count() << " file opened since the other is out of scope" << endl;

	return 0;
}
