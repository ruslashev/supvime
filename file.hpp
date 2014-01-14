#ifndef FILE_HPP
#define FILE_HPP

#include <string>
#include <fstream>
#include <vector>

class File
{
public:
	std::vector<std::string> lines;
	std::string filename;

	void Open(std::string fileToOpen);
};

#endif
