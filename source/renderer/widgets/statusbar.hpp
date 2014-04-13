#ifndef STATUSBAR_HPP
#define STATUSBAR_HPP

#include "../basedrawablewidget.hpp"
#include "../textdrawer.hpp"
#include "../mesh.hpp"

class StatusBar : public BaseDrawableWidget
{
	TextDrawer td;
	Mesh modeMesh;
public:
	StatusBar(const char *fontPath);
	void Draw();
};

#endif


