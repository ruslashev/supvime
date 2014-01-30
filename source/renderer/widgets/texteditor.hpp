#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

#include "../../editor.hpp"
#include "../basedrawablewidget.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <GL/glew.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

class TextEditor : public BaseDrawableWidget
{
	FT_Library ft;
	FT_Face fontFace;
	GLuint fg_textVBO, bg_textVBO;
	GLint fg_textureUnif, fg_FGcolorUnif, bg_BGcolorUnif;
	GLint fg_coordAttribute, bg_vcoordAttribute;
	GLuint fg_vertShader, fg_fragShader, bg_vertShader, bg_fragShader;
	GLuint fg_shaderProgram, bg_shaderProgram;
	unsigned int fontHeight;
	const float sx, sy;
	FT_GlyphSlot g;

	void InitGL();
	void RenderFile();
	void RenderChar(const uint32_t ch, float &dx, const float dy, const float adv, const float vadv, const int cx);
	// void RenderString(const char *text, int x, int y);
	void setTextForeground(unsigned char r, unsigned char g, unsigned char b);
	void setTextBackground(unsigned char r, unsigned char g, unsigned char b);
	void setTextSize(unsigned int size);
public:
	TextEditor(const char *fontPath);
	~TextEditor();

	void Draw();
};

#endif

