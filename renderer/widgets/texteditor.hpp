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
	// float fontWidth, fontHeight;
	SDL_Window *wp;
	FT_Library ft;
	FT_Face fontFace;
	GLuint textVBO;
	GLint fontTextureUnif, fontFGcolorUnif, fontBGcolorUnif;
	GLint text_coordAttribute;
	GLuint vertShader, fragShader;
	GLuint shaderProgram;

	void InitGL();
	void RenderText(const char *text, float x, float y, float scaleX, float scaleY);
	GLuint CreateShader(GLenum type, const char *src);
	GLuint CreateShaderProgram(GLuint vs, GLuint fs);
	GLint BindUniform(GLuint shaderProgramP, const char *name);
	void PrintLog(GLuint &shaderOrProg);
	void setTextForeground(unsigned char r, unsigned char g, unsigned char b);
	void setTextBackground(unsigned char r, unsigned char g, unsigned char b);
	void setTextSize(unsigned int height);
	// void markBlock(int sy, int sx, int ey, int ex);
public:
	// SDL_Texture *texture;
	std::vector<Line> *lines;
	SDL_Rect pos;

	TextEditor(const char *fontPath, SDL_Rect npos, SDL_Window *nwp);
	~TextEditor();

	void Draw();
};

#endif

