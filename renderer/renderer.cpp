#include "renderer.hpp"

Renderer::Renderer(Editor *nep)
{
	ep = nep;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Failed to initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	// Initialize SDL_ttf
	if (TTF_Init() == -1) {
		printf("Failed to initialize SDL_ttf: %s\n", TTF_GetError());
		exit(1);
	}

	// Create window
	window = SDL_CreateWindow("Supvime loading..",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600,
			SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Failed to open a window: %s\n", SDL_GetError());
		exit(1);
	}

	// Create an SDL renderer
	renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("Failed to create a renderer: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

	widgets.push_back(std::unique_ptr<TextEditor>(new TextEditor(80, 25, "DroidSansMono.ttf", 13, renderer, ep)));
}

void Renderer::UpdateTitle()
{
	char titleBuf[128];
	snprintf(titleBuf, 128, "%s <modified?> - Supvime", ep->fp->filename.c_str());
	SDL_SetWindowTitle(window, titleBuf);
}

void Renderer::Update(std::vector<std::string> &nlines)
{
	SDL_RenderClear(renderer);

	widgets[0]->lines = nlines;
	for (auto &w : widgets) {
		w->Draw();
	}

	SDL_RenderPresent(renderer);

	UpdateTitle();
}

char Renderer::getch()
{
	SDL_StartTextInput();
	while (1) {
		if (SDL_PollEvent(&event)) {
			if (event.type == SDL_TEXTINPUT) {
				return event.text.text[0];
			} else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_TAB:
						return 9;
						break;
					default:
						break;
				}
			}
		}
	}
}

Renderer::~Renderer()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}

