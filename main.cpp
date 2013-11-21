#include <fstream>
#include "file.hpp"
#include "renderer.hpp"
#include "editor.hpp"

int main()
{
	File file("file.cpp");
	Editor ed(&file);
	Renderer rend(&ed, 80, 25, "DroidSansMono.ttf", 13);

	while (1) {
		rend.Redraw(file.lines);
		ed.ProcessKey(rend.getch());
	}

	return 0;
}

