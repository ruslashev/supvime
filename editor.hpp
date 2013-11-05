#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <ncurses.h>
#include "file.hpp"
#include "renderer.hpp"

class Editor
{
	File *fp; // File to operate on
	Renderer *rp;

	void ProcessNormalModeKey(int key);
	void ProcessInsertModeKey(int key);
public:
	enum { NORMAL, INSERT } mode;

	Editor(File *nfp, Renderer *nrp);
	void ProcessKey(int key);
};

#endif

