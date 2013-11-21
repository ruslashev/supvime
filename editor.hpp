#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "file.hpp"

class Editor
{
	File *fp; // File to operate on

	void ProcessNormalModeKey(int key);
	void ProcessInsertModeKey(int key);
public:
	enum { NORMAL, INSERT } mode;
	struct { unsigned int x, y; } curs;
	unsigned int oldColumn;

	Editor(File *nfp);
	void ProcessKey(int key);
};

#endif

