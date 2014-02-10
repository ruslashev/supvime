/* TODO:
 * Get rid of all todos
 * Create separate text renderer class
*/

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
		file.Open("source/file.cpp");
		ed.Load(&file);
		rend.Create(&ed);
	} catch (std::exception &e) {
		fprintf(stderr, "\x1b[31m" "ERROR" "\x1b[0m" " %s\n", e.what());
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

