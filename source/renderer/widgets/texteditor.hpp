#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

#include "../basedrawablewidget.hpp"
#include "../textdrawer.hpp"

class TextEditor : public BaseDrawableWidget
{
	TextDrawer td;
	void RenderFile();
public:
	TextEditor(const char *fontPath);

	void Draw();
};

#endif

