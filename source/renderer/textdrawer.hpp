#ifndef TEXTDRAWER_HPP
#define TEXTDRAWER_HPP

#include <map>

#include <GL/glew.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

struct glyph {
	GLuint fg_glyphVertCoordsVBO, textureID;
	long xAdvance;
	int left, top, width, height;
};
struct glyphKey {
	uint32_t ch;
	unsigned int size;
	bool operator<(const glyphKey &other) const {
		return ch < other.ch;
	}
};

class TextDrawer;

class TextCacher
{
	std::map<glyphKey, glyph> normalGlyphs;
public:
	FT_Face face;
	FT_Library ftLib;
	TextDrawer *ted;

	GLuint fg_texCoordsVBO, bg_cellVertCoordsVBO;

	void Precache(unsigned int size);
	glyph Lookup(uint32_t ch, unsigned int size);
	~TextCacher();
};

class TextDrawer
{
	FT_Library ftLib;
	FT_Face mainFace;
	TextCacher cacher;

	GLint fg_textureUnif, fg_FGcolorUnif, fg_transfUnif, fg_gtransfUnif;
	GLint fg_vertCoordAttribute, fg_textureCoordAttribute;
	GLuint fg_vertShader, fg_fragShader;
	GLuint fg_shaderProgram;

	GLint bg_BGcolorUnif, bg_transfUnif, bg_gtransfUnif;
	GLint bg_vertCoordAttribute;
	GLuint bg_vertShader, bg_fragShader;
	GLuint bg_shaderProgram;

	void InitGL();
public:
	TextDrawer(const char *fontPath);
	~TextDrawer();

	unsigned int fontHeight;
	const float sx, sy;
	const float lineSpacing;

	void RenderChar(const uint32_t ch, float &dx, const float dy, const int cx);
	void setTextForeground(unsigned char r, unsigned char g, unsigned char b);
	void setTextBackground(unsigned char r, unsigned char g, unsigned char b);
	void setGlobalTransformation(float x, float y);
	void setTextSize(unsigned int size);
};

#endif

