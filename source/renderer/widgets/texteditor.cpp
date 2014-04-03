#include "texteditor.hpp"
#include "../textdrawer.hpp"

TextEditor::TextEditor(const char *fontPath) : td(fontPath)
{
}

void TextEditor::Draw()
{
	td.setTextSize(14);
	td.setTextForeground(0, 0, 0);
	td.setTextBackground(255, 255, 255);

	RenderFile();

	for (size_t i = 0; i < ep->lines.size(); i++)
		ep->lines.at(i).dirty = false;
}

void TextEditor::RenderFile()
{
	const float cellHeight = (int)(td.fontHeight*td.lineSpacing)*td.sy;

	for (size_t l = 0; l < ep->lines.size(); l++) {
		float dx = -1;
		const float dy = 1 - l*cellHeight;
		const std::string srcLine = ep->lines.at(l).str;

		int cx = 0;
		for (size_t c = 0; c < srcLine.length(); c++, cx++) {
			// TODO
			if (l == ep->curs.y && c == ep->curs.x)
				td.setTextBackground(100, 100, 100);
			else
				td.setTextBackground(255, 255, 255);

			if (srcLine[c] != '\t')
				td.RenderChar(srcLine[c], dx, dy);
			else {
				const int tabsize = 4;
				const int spacesToInsert = tabsize - (cx % tabsize);
				td.setTextForeground(200, 200, 200);
				td.RenderChar('|', dx, dy);
				cx++;
				for (int i = 1; i < spacesToInsert; i++) {
					td.RenderChar('-', dx, dy);
					cx++;
				}
				cx--;
				td.setTextForeground(0, 0, 0);
			}
		}
	}
}

