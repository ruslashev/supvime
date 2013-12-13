#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "editor.hpp"

struct Cell
{
	std::string ch;
	// string because `char`s aren't sufficient
	// and `char*`s are cumbersome
	uint8_t flags;
	// 000C VBIU
	// V - inverse
	// B - bold
	// I - italics
	// U - underline
	// C - color on
	uint8_t color;
};

class Renderer
{
private:
	Editor *ep;

	TTF_Font *font;
	std::vector<std::vector<Cell>> screen;

	int fontWidth, fontHeight;

	void UpdateTitle();
	void RebuildSurface();

	void move(int y, int x);
	void addch(std::string c);
	void addstr(std::string str);
	void mvaddch(int y, int x, std::string c);
	void mvaddstr(int y, int x, std::string str);

	void markBlock(int sy, int sx, int ey, int ex);

	void clear();

	struct { unsigned int x, y; } drwCurs; // cursor used for drawing, not visible
public:
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	int cols, rows;

	Renderer(Editor *nep, int ncols, int nrows, const char *fontPath, int fontSize);
	~Renderer();

	void Update(std::vector<std::string> &lines);
	char getch();
};

#endif
