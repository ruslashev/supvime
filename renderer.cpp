#include "renderer.hpp"

Renderer::Renderer()
{
	initscr();
	raw();
	noecho();
	keypad(stdscr, true);

	getmaxyx(stdscr, ROWS, COLS);
}

void Renderer::Redraw(std::vector<std::string> lines, int x, int y)
{
	int i = 0;
	for (auto &s : lines) {
		mvprintw(i, 0, "%3d  %s\n", i+1, s.c_str());
		i++;
	}
	while (i < ROWS-1)
		mvaddstr(i++, 0, "~");
	refresh();

	move(y, x+5);
}

