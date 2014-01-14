#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "widgets/texteditor.hpp"
#include "../editor.hpp"

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
	SDL_GLContext ctxt;
	SDL_Event event;
	std::vector<std::unique_ptr<TextEditor>> widgets; // BaseDrawableWidget> widgets;

	void UpdateTitle();
public:
	void Create(Editor *nep);
	~Renderer();

	void Update();
	char getch();
};

#endif

