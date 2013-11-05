#include <fstream>
#include "file.hpp"
#include "renderer.hpp"
#include "editor.hpp"

int main()
{
	File file("file.cpp");
	Renderer rend;
	Editor ed(&file, &rend);

	rend.cursor.x = rend.cursor.y = 0;

	while (1) {
		rend.Redraw(file.lines);
		ed.ProcessKey(getch());
	}

	endwin();
	return 0;
}

