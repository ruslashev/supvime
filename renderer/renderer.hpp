#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "../editor.hpp"
#include "widgets/texteditor.hpp"

#include <vector>
#include <string>
#include <memory>
#include <SDL2/SDL.h>

class BaseDrawableWidget
{
public:
	SDL_Renderer *rend;

	void Draw();
};

class Renderer
{
private:
	Editor *ep;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event event;
	std::vector<std::unique_ptr<TextEditor>> widgets; // BaseDrawableWidget> widgets;

	void UpdateTitle();
public:
	Renderer(Editor *nep);
	~Renderer();

	void Update(std::vector<std::string> &nlines); // includes redrawing
	char getch();
};

#endif

