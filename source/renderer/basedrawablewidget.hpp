#ifndef BASEDRAWABLEWIDGET_HPP
#define BASEDRAWABLEWIDGET_HPP

#include "../editor.hpp"
#include "textdrawer.hpp"

class BaseDrawableWidget
{
public:
	Editor *ep;
	TextDrawer *td;
	virtual void Draw() = 0;
};

#endif

