#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "file.hpp"

struct Line
{
	bool dirty;
	std::string str;
};

class Editor
{
	void ProcessNormalModeKey(int key);
	void ProcessInsertModeKey(int key);

	unsigned int oldColumn;
public:
	File *fp; // File to operate on
	bool quit;
	std::vector<Line> lines;

	enum { NORMAL, INSERT } mode;
	struct { unsigned int x, y; } curs;

	Editor(File *nfp);
	void ProcessKey(int key);
};

#endif

