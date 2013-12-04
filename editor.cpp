#include "editor.hpp"

Editor::Editor(File *nfp)
{
	fp = nfp;
	mode = NORMAL;

	curs.x = curs.y = oldColumn = 0;
}

void Editor::ProcessKey(int key)
{
	printf("Editor::ProcessKey(%d)\n", key);

	switch (mode) {
		case NORMAL:
			ProcessNormalModeKey(key);
			break;
		case INSERT:
			ProcessInsertModeKey(key);
			break;
	}
}

void Editor::ProcessNormalModeKey(int key)
{
	switch (key) {
		case 'i':
			mode = INSERT;
			break;
		case 'q':
			exit(0);
			break;
		case 'h':
			if (curs.x > 0)
				oldColumn = --curs.x;
			break;
		case 'l':
			if (curs.x < fp->lines[curs.y].size()-1)
				oldColumn = ++curs.x;
			break;
		case 'k':
			if (curs.y > 0)
				curs.y--;
			break;
		case 'j':
			if (curs.y < fp->lines.size()-1)
				curs.y++;
			break;
	}
}

void Editor::ProcessInsertModeKey(int key)
{
	switch (key) {
		case 9: // Tab
			mode = NORMAL;
			break;
		default:
			if (32 <= key && key <= 126) {
				fp->lines[curs.y].insert(curs.x, 1, (char)key);
				curs.x++;
			}
	}
}

