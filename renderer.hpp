#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <string>
#include <ncurses.h>
#include "editor.hpp"

class Renderer
{
	Editor *ep;
public:
	int COLS, ROWS;
	Renderer(Editor *nep);
	void Redraw(std::vector<std::string> lines);
};

#endif
