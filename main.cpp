#include "file.hpp"
#include "renderer/renderer.hpp"
#include "editor.hpp"

#include <fstream>

int main()
{
	// TODO throw exceptions instead of `exit(..)`
	File file("file.cpp");
	Editor ed(&file);
	Renderer rend(&ed);

	while (1) {
		rend.Update();
		ed.ProcessKey(rend.getch());
		if (ed.quit)
			break;
	}

	return 0;
}

