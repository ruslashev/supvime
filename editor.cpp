#include "editor.hpp"

Editor::Editor(File *nfp, Renderer *nrp)
{
	fp = nfp;
	rp = nrp;
	mode = NORMAL;
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
			endwin();
			exit(0);
			break;
		case 'h':
			if (rp->curs.x > 0)
				rp->curs.x--;
			break;
		case 'l':
			if (rp->curs.x < fp->lines[rp->curs.y].size()-1)
				rp->curs.x++;
			break;
		case 'k':
			if (rp->curs.y > 0)
				rp->curs.y--;
			break;
		case 'j':
			if (rp->curs.y < fp->lines.size()-1)
				rp->curs.y++;
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
				fp->lines[rp->curs.y].insert(rp->curs.x, 1, (char)key);
				rp->curs.x++;
			}
	}
}

