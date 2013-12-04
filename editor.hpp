#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "file.hpp"

class Editor
{
	void ProcessNormalModeKey(int key);
	void ProcessInsertModeKey(int key);
public:
	File *fp; // File to operate on

	enum { NORMAL, INSERT } mode;
	struct { unsigned int x, y; } curs;
	unsigned int oldColumn;

	Editor(File *nfp);
	void ProcessKey(int key);
};

#endif

