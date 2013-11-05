#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <string>
#include <ncurses.h>

class Renderer
{
public:
	struct { unsigned int x, y; } curs;
	int COLS, ROWS;
	Renderer();
	void Redraw(std::vector<std::string> lines);
};

#endif
