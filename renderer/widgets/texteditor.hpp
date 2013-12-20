#ifndef TEXTEDITOR_HPP
#define TEXTEDITOR_HPP

#include "../../editor.hpp"

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

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

class TextEditor// : public BaseDrawableWidget
{
private:
	SDL_Surface *textAreaSurf;
	SDL_Texture *texture;
	TTF_Font *font;
	std::vector<std::vector<Cell>> screen;
	struct { unsigned int x, y; } drwCurs; // cursor used for drawing, not visible
	int fontWidth, fontHeight;
	int cols, rows;
	SDL_Renderer *rend;
	Editor *ep;

	void RebuildSurface();
	void move(int y, int x);
	void addch(std::string c);
	void addstr(std::string str);
	void mvaddch(int y, int x, std::string c);
	void mvaddstr(int y, int x, std::string str);
	void clear();
	void markBlock(int sy, int sx, int ey, int ex);
public:
	std::vector<std::string> lines;

	TextEditor(int ncols, int nrows, const char *fontPath, int fontSize, SDL_Renderer *nrend, Editor *nep);
	~TextEditor();

	void Draw();
};

#endif

