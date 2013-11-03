#include <fstream>
#include "file.hpp"

int main()
{
	File file("file.cpp");

	for (auto &s : file.lines)
		printf("%s\n", s.c_str());

	return 0;
}

