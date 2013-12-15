#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <vector>
#include <string>
#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "editor.hpp"

struct Cell
{
	std::string ch;
	// string because `char`s aren't sufficient for other languages
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

class BaseDrawableWidget
{
public:
	SDL_Renderer *rend;

	void Draw();
};

class TextEditor// : public BaseDrawableWidget
{
private:
	SDL_Surface *textAreaSurf;
	SDL_Texture *textAreaTexture;
	TTF_Font *font;
	std::vector<std::vector<Cell>> screen;
	struct { unsigned int x, y; } drwCurs; // cursor used for drawing, not visible
	int fontWidth, fontHeight;
	int cols, rows;

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
	SDL_Renderer *rend;
	Editor *ep;

	TextEditor(int ncols, int nrows, const char *fontPath, int fontSize, SDL_Renderer *nrend);
	~TextEditor();

	void Draw();
};

class Renderer
{
private:
	Editor *ep;
	std::vector<std::unique_ptr<TextEditor>> widgets; // BaseDrawableWidget> widgets;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;

	void UpdateTitle();
public:
	Renderer(Editor *nep);
	~Renderer();

	void Update(std::vector<std::string> &lines); // includes redrawing
	char getch();
};

#endif
