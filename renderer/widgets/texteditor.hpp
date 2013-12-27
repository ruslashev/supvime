#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

// #include "../renderer.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

extern "C" {
#include "../../font-stash/fontstash.h"
}

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
	struct sth_stash *stash;
	int font;
	unsigned char *fontData;
	std::vector<Row> screen;
	struct { unsigned int x, y; } drwCurs; // cursor used for drawing, not visible
	int fontWidth, fontHeight;
	int cols, rows;
	SDL_Window *wp;

	void RebuildSurface();
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

	TextEditor(int ncols, int nrows, const char *fontPath, int fontSize, SDL_Window *nwp);
	~TextEditor();

	void Draw();
};

#endif

