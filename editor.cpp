#include "editor.hpp"

void Editor::Load(File *nfp)
{
	fp = nfp;
	mode = NORMAL;
	quit = false;

	curs.x = curs.y = oldColumn = 0;

	lines.reserve(fp->lines.size());
	for (std::string &str : fp->lines)
		lines.push_back(Line { /* dirty: */ true, /* str: */ str });
}

void Editor::ProcessKey(int key)
{
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
			quit = true;
			break;
		case 'h':
			if (curs.x > 0)
				oldColumn = --curs.x;
			break;
		case 'l':
			if (curs.x < lines[curs.y].str.size()-1)
				oldColumn = ++curs.x;
			break;
		case 'k':
			if (curs.y > 0)
				curs.y--;
			break;
		case 'j':
			if (curs.y < lines.size()-1)
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
				lines[curs.y].str.insert(curs.x, 1, (char)key);
				lines[curs.y].dirty = true;
				curs.x++;
			}
	}
}

