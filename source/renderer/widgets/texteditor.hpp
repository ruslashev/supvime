#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

#include "../../editor.hpp"
#include "../basedrawablewidget.hpp"

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <fstream>

#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

struct glyph_t {
	GLuint textureID;
	long xAdvance;
	int left, top, width, height;
};
struct glyphKey_t {
	uint32_t ch;
	unsigned int size;
	bool operator<(const glyphKey_t &other) const {
		return ch < other.ch;
	}
};

class TextCacher
{
	std::map<glyphKey_t, glyph_t> normalGlyphs;
public:
	FT_Face face;
	FT_Library ftLib;
	glyph_t Lookup(uint32_t ch, unsigned int size);
	~TextCacher();
};

class TextEditor : public BaseDrawableWidget
{
	FT_Library ftLib;
	FT_Face mainFace;
	unsigned int fontHeight;
	TextCacher cacher;

	GLuint fg_textVBO, bg_textVBO;
	GLint fg_textureUnif, fg_FGcolorUnif, bg_BGcolorUnif;
	GLint fg_coordAttribute, bg_vcoordAttribute;
	GLuint fg_vertShader, fg_fragShader, bg_vertShader, bg_fragShader;
	GLuint fg_shaderProgram, bg_shaderProgram;

	const float sx, sy;
	const float lineSpacing;

	void InitGL();
	void RenderFile();
	void RenderChar(const uint32_t ch, float &dx, const float dy, const int cx);
	void setTextForeground(unsigned char r, unsigned char g, unsigned char b);
	void setTextBackground(unsigned char r, unsigned char g, unsigned char b);
	void setTextSize(unsigned int size);
public:
	TextEditor(const char *fontPath);
	~TextEditor();

	void Draw();
};

#endif

