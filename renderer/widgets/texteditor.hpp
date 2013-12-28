#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

// #include "../renderer.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#ifndef RENDERER_HPP
#define FONTSTASH_IMPLEMENTATION
#include "../../fontstash/fontstash.h"
#define GLFONTSTASH_IMPLEMENTATION
#include "../../fontstash/glfontstash.h"
#endif

struct Cell
{
	std::string ch;
	// string because `char`s aren't sufficient for other languages
	// and `char*`s are cumbersome
	uint8_t flags;
	// 000C BIUV
	// C - color on
	// B - bold
	// I - italics
	// U - underline
	// V - inverse
	uint8_t color;
};

struct Row
{
	bool dirty;
	std::vector<Cell> cells;

	Cell& operator[](size_t i) { return this->cells[i]; }
	// const Cell& operator[](size_t i) const { return this->cells[i]; }
};

class TextEditor// : public BaseDrawableWidget
{
private:
	struct FONScontext *stash;
	int font;
	std::vector<Row> screen;
	struct { unsigned int x, y; } drwCurs; // cursor used for drawing, not visible
	float fontWidth, fontHeight;
	int cols, rows;
	SDL_Window *wp;

	void move(int y, int x);
	void addch(std::string c);
	void addstr(std::string str);
	void mvaddch(int y, int x, std::string c);
	void mvaddstr(int y, int x, std::string str);
	void clear();
	void markBlock(int sy, int sx, int ey, int ex);
public:
	// SDL_Texture *texture;
	std::vector<std::string> lines;
	SDL_Rect pos;

	TextEditor(int ncols, int nrows, const char *fontPath, SDL_Rect npos, SDL_Window *nwp);
	~TextEditor();

	void Draw();
};

#endif

