#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

#include "../../editor.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <GL/glew.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

/*
struct Cell
{
	std::string ch;
	// string because `char`s aren't sufficient for other languages
	// and `char*`s are cumbersome
};

struct CellFlag
{
	uint8_t flag;
	// CCCC BIUV
	// C - palette color index
	// B - bold
	// I - italics
	// U - underline
	// V - inverse
	size_t start; // On what column does flag start
	size_t len;   // how many lines spans
};

struct DrawableLine
{
	std::string str;
	std::vector<CellFlag> flags;
};
*/

class TextEditor// : public BaseDrawableWidget
{
private:
	FT_Library ft;
	FT_Face fontFace;
	GLuint fg_textVBO, bg_textVBO;
	GLint fg_textureUnif, fg_FGcolorUnif, bg_BGcolorUnif;
	GLint fg_coordAttribute, bg_vcoordAttribute;
	GLuint fgVertShader, fgFragShader, bgVertShader, bgFragShader;
	GLuint fgShaderProgram, bgShaderProgram;
	unsigned int fontHeight;

	void InitGL();
	void RenderFile(const float sx, float sy);
	void RenderString(const char *text, int x, int y, const float sx, float sy);
	void setTextForeground(unsigned char r, unsigned char g, unsigned char b);
	void setTextBackground(unsigned char r, unsigned char g, unsigned char b);
	void setTextSize(unsigned int size);
public:
	std::vector<Line> *lines;

	TextEditor(const char *fontPath);
	~TextEditor();

	void Draw();
};

#endif

