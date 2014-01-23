#ifndef BASEDRAWABLEWIDGET_HPP
#define BASEDRAWABLEWIDGET_HPP

#include "../editor.hpp"

class BaseDrawableWidget
{
public:
	Editor *ep;
	virtual void Draw() = 0;
};

#endif

