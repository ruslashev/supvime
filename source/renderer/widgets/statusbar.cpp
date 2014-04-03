#include "statusbar.hpp"

StatusBar::StatusBar(const char *fontPath) : td(fontPath)
{
}

void StatusBar::Draw()
{
	td.setTextSize(12);
	td.setTextForeground(255, 255, 255);
	td.setTextBackground(50, 50, 50);

	const float margin = 3;
	float dx = -1+margin*td.sx;
	const std::string mode = (ep->mode == Editor::NORMAL ? "NORMAL" : "INSERT");
	td.RenderString(mode, dx, -1+td.fontHeight*td.lineSpacing*td.sy+margin*td.sy);
}

