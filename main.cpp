#include <fstream>
#include "file.hpp"
#include "renderer.hpp"

int main()
{
	Renderer rend;
	File file("file.cpp");

	struct { unsigned int x, y; } cursor;
	cursor.x = cursor.y = 0;

	while (1) {
		rend.Redraw(file.lines, cursor.x, cursor.y);

		int c = getch();
		if (c == 'q') {
			break;
		} else if (c == 'h') {
			if (cursor.x > 0)
				cursor.x--;
		} else if (c == 'l') {
			if (cursor.x < file.lines[cursor.y].size()-1)
				cursor.x++;
		} else if (c == 'j') {
			if (cursor.y < file.lines.size()-1)
				cursor.y++;
		} else if (c == 'k') {
			if (cursor.y > 0)
				cursor.y--;
		}
	}

	endwin();
	return 0;
}

