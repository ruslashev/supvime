#include <fstream>
#include "file.hpp"
#include "renderer.hpp"
#include "editor.hpp"

int main()
{
	File file("file.cpp");
	Editor ed(&file);
	Renderer rend(&ed);

	while (1) {
		rend.Redraw(file.lines);
		ed.ProcessKey(getch());
	}

	endwin();
	return 0;
}

