#include "renderer.hpp"
#include "../errors.hpp"

void Renderer::Create(Editor *nep)
{
	ep = nep;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		throwf("Failed to initialize SDL: %s\n", SDL_GetError());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);

	// Create window
	window = SDL_CreateWindow("Supvime loading..",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == NULL)
		throwf("Failed to open a window: %s\n", SDL_GetError());

	// Initialize SDL_GL
	ctxt = SDL_GL_CreateContext(window);
	if (ctxt == NULL)
		throwf("Failed to create OpenGL rendering context: %s\n", SDL_GetError());
	if (SDL_GL_SetSwapInterval(1) < 0)
		printf("Warning: Unable to set VSync: %s\n", SDL_GetError());

	/*
	// Create an SDL renderer
	renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
		throwf("Failed to create a renderer: %s\n", SDL_GetError());
	*/

	widgets.push_back(std::unique_ptr<TextEditor>(
				new TextEditor("symlink-to-font")));
	widgets[0]->lines = &ep->lines;
}

void Renderer::UpdateTitle()
{
	char titleBuf[128];
	snprintf(titleBuf, 128, "%s <modified?> - Supvime", ep->fp->filename.c_str());
	SDL_SetWindowTitle(window, titleBuf);
}

void Renderer::Update()
{
	glViewport(0, 0, 800, 600);

	const float grayShade = 0.075f;
	glClearColor(grayShade, grayShade, grayShade, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (auto &w : widgets) {
		w->Draw();
	}

	SDL_GL_SwapWindow(window);

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
	// SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

