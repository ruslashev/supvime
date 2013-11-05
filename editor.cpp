#include "editor.hpp"

Editor::Editor(File *nfp, Renderer *nrp)
{
	fp = nfp;
	rp = nrp;
}

void Editor::ProcessKey(int key)
{
	if (key == 'q') {
		endwin();
		exit(0);
	} else if (key == 'h') {
		if (rp->cursor.x > 0)
			rp->cursor.x--;
	} else if (key == 'l') {
		if (rp->cursor.x < fp->lines[rp->cursor.y].size()-1)
			rp->cursor.x++;
	} else if (key == 'j') {
		if (rp->cursor.y < fp->lines.size()-1)
			rp->cursor.y++;
	} else if (key == 'k') {
		if (rp->cursor.y > 0)
			rp->cursor.y--;
	}
}
