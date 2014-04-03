#include "statusbar.hpp"

StatusBar::StatusBar(const char *fontPath) : td(fontPath)
{
}

void StatusBar::Draw()
{
	td.setTextSize(14);
	td.setTextForeground(255, 255, 255);
	td.setTextBackground(90, 90, 90);

	const float margin = 1;
	float dx = -1+margin*td.sx;
	float dy = -1 + td.fontHeight*td.lineSpacing*td.sy + margin*td.sy;
	std::string mode = (ep->mode == Editor::NORMAL ? " NORMAL " : " INSERT ");
	td.RenderString(mode, dx, dy);

	td.setTextBackground(50, 50, 50);
	td.RenderString(" ", dx, dy);
	td.RenderString(ep->fp->filename, dx, dy);
	td.RenderString(" ", dx, dy);
}

