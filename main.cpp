#include "file.hpp"
#include "renderer/renderer.hpp"
#include "editor.hpp"

#include <fstream>

int main()
{
	File file;
	Editor ed;
	Renderer rend;
	try {
		file.Open("file.cpp");
		ed.Load(&file);
		rend.Create(&ed);
	} catch (...) {
		return 1;
	}

	while (1) {
		rend.Update();
		ed.ProcessKey(rend.getch());
		if (ed.quit)
			break;
	}

	return 0;
}

