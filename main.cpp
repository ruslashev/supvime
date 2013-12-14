#include <fstream>
#include "file.hpp"
#include "renderer.hpp"
#include "editor.hpp"

int main()
{
	File file("file.cpp");
	Editor ed(&file);
	Renderer rend(&ed);
	rend.Update(file.lines);

	while (1) {
		rend.Update(file.lines);
		ed.ProcessKey(rend.getch());
	}

	return 0;
}

