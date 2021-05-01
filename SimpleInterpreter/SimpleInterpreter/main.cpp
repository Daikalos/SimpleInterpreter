#include "Interpreter.h"

int main()
{
	Interpreter interp(std::cout);

	interp.read_file("codefile.txt");
	std::cin.get();

	interp.clear(); // Reset all private members to prevent leakage

	std::cout << "type code for interpreter; type EOF to stop\n\n";

	interp.read_stream(std::cin);
	std::cin.get();

	return 0;
}