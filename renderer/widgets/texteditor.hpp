#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

#include "../../editor.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

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
	SDL_Window *wp;

	FT_Library ft;
	FT_Face fontFace;
	GLuint fg_textVBO, bg_textVBO;
	GLint fg_textureUnif, fg_FGcolorUnif, bg_BGcolorUnif;
	GLint fg_coordAttribute, bg_vcoordAttribute;
	GLuint fgVertShader, fgFragShader, bgVertShader, bgFragShader;
	GLuint fgShaderProgram, bgShaderProgram;
	unsigned int cellHeight;

	void InitGL();
	void RenderText(const char *text, int x, int y, const float sx, float sy);
	GLuint CreateShader(GLenum type, const char *src);
	GLuint CreateShaderProgram(GLuint vs, GLuint fs);
	GLint BindUniform(GLuint shaderProgramP, const char *name);
	GLint BindAttribute(GLuint shaderProgramP, const char *name);
	void PrintLog(GLuint &shaderOrProg);
	void setTextForeground(unsigned char r, unsigned char g, unsigned char b);
	void setTextBackground(unsigned char r, unsigned char g, unsigned char b);
	void setTextSize(unsigned int size);
public:
	std::vector<Line> *lines;
	SDL_Rect pos;

	TextEditor(const char *fontPath, SDL_Rect npos, SDL_Window *nwp);
	~TextEditor();

	void Draw();
};

#endif

