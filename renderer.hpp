#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <string>
#include <ncurses.h>

class Renderer
{
public:
	int COLS, ROWS;
	Renderer();
	void Redraw(std::vector<std::string> lines, int x, int y);
};

#endif
