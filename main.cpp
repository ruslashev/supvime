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
	} catch (std::exception &e) {
		printf("\x1b[31m" "ERROR" "\x1b[0m" " %s\n", e.what());
		return 1;
	} catch (...) {
		puts("Unknown error");
		return 2;
	}

	while (1) {
		rend.Update();
		ed.ProcessKey(rend.getch());
		if (ed.quit)
			break;
	}

	return 0;
}

