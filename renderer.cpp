#include "renderer.hpp"

Renderer::Renderer()
{
	initscr();
	raw();
	noecho();
	keypad(stdscr, true);

	getmaxyx(stdscr, ROWS, COLS);
}

void Renderer::Redraw(std::vector<std::string> lines)
{
	int i = 0;
	for (; i < (int)lines.size(); i++)
		mvprintw(i, 0, "%3d  %s\n", i+1, lines[i].c_str());
	for (; i < ROWS-1; i++)
		mvaddstr(i, 0, "~");
	refresh();

	move(curs.y, curs.x+5);
}

