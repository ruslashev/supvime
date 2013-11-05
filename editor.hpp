#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <ncurses.h>
#include "file.hpp"
#include "renderer.hpp"

class Editor
{
public:
	File *fp; // File to operate on
	Renderer *rp;
	Editor(File *nfp, Renderer *nrp);
	void ProcessKey(int key);
};

#endif

