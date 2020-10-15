#include "Interpreter.h"

int main()
{
	Interpreter intrpr(std::cout);

	intrpr.read_stream(std::cin);
	std::cin.get();

	return 0;
}