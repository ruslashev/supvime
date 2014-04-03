#ifndef STATUSBAR_HPP
#define STATUSBAR_HPP

#include "../basedrawablewidget.hpp"
#include "../textdrawer.hpp"

class StatusBar : public BaseDrawableWidget
{
	TextDrawer td;
public:
	StatusBar(const char *fontPath);
	void Draw();
};

#endif


