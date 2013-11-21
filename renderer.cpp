#include "renderer.hpp"

Renderer::Renderer(Editor *nep, int ncols, int nrows, const char *fontPath, int fontSize)
{
	ep = nep;
	cols = ncols;
	rows = nrows;

	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("Failed to Initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}

	// Initialize SDL_ttf
	if (TTF_Init() == -1) {
		printf("Failed to Initialize SDL_ttf: %s\n", TTF_GetError());
		exit(1);
	}

	// Open font
	font = TTF_OpenFont(fontPath, fontSize);
	if (font == NULL) {
		printf("TTF error: %s\n", TTF_GetError());
		exit(2);
	}

	// Get font size
	int fontWidth, fontHeight = TTF_FontHeight(font);
	TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &fontWidth);

	// Create window
	window = SDL_CreateWindow("Supvime loading..",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			cols*fontWidth, rows*fontHeight,
			SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Failed to open a window: %s\n", SDL_GetError());
		exit(1);
	}

	// Create an SDL renderer
	renderer = SDL_CreateRenderer(window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		printf("Failed to initialize a renderer: %s\n", SDL_GetError());
		exit(1);
	}

	// TODO move this up
	TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

	screen.resize(rows);
	for (int y = 0; y < rows; y++)
		screen[y].resize(cols);
	clear();

	RebuildSurface();
	UpdateTitle();
}

Renderer::~Renderer()
{
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}

void Renderer::RebuildSurface()
{
	std::string rowStr;
	SDL_Surface *fontSurf;
	SDL_Texture *screenTexture;
	for (int y = 0; y < rows; y++)
	{
		rowStr.clear();
		for (std::string &str : screen[y])
			rowStr += str;

		fontSurf = TTF_RenderUTF8_Shaded(font, rowStr.c_str(),
				{255, 255, 255, 255}, {20, 20, 20, 255});
		SDL_Rect offsetRect = { 0, fontSurf->h*y, fontSurf->w, fontSurf->h };
		screenTexture = SDL_CreateTextureFromSurface(renderer, fontSurf);
		SDL_RenderCopy(renderer, screenTexture, NULL, &offsetRect);
		SDL_FreeSurface(fontSurf);
		SDL_DestroyTexture(screenTexture);
	}
}

void Renderer::Redraw(std::vector<std::string> &lines)
{
	// int i = 0;
	// for (; i < (int)lines.size(); i++)
	// 	mvprintw(i, 0, "%3d  %s\n", i+1, lines[i].c_str());
	// for (; i < rows-1; i++)
	// 	mvaddstr(i, 0, "~");

	// move(ep->curs.y, ep->curs.x+5);

	int i = 0;
	for (; i < (int)lines.size(); i++)
		mvaddstr(i, 0, lines[i]);
	for (; i < rows-1; i++)
		mvaddstr(i, 0, "~");

	RebuildSurface();

	SDL_RenderPresent(renderer);
}

void Renderer::clear()
{
	for (int y = 0; y < rows; y++)
		for (int x = 0; x < cols; x++)
			screen[y][x] = " ";
}
void Renderer::move(int y, int x)
{
	drwCurs = { (unsigned int)x, (unsigned int)y };
}
void Renderer::addch(std::string c)
{
	if ((int)drwCurs.x <= cols-1 && (int)drwCurs.y <= rows-1)
		screen[drwCurs.y][drwCurs.x].assign(c);
}
void Renderer::addstr(std::string str)
{
	for (int x = drwCurs.x; x < (int)drwCurs.x+(int)str.length(); x++) {
		if (x >= cols)
			break;
		screen[drwCurs.y][x] = str[x-drwCurs.x];
	}
}
void Renderer::mvaddch(int y, int x, std::string c)
{
	move(y, x);
	addch(c);
}
void Renderer::mvaddstr(int y, int x, std::string str)
{
	move(y, x);
	addstr(str);
}
char Renderer::getch()
{
	SDL_Event event;
	SDL_WaitEvent(&event);
	if (event.type == SDL_KEYDOWN)
		return (char)event.key.keysym.unicode;
	else
		return 0;
}

void Renderer::UpdateTitle()
{
	char titleBuf[128];
	snprintf(titleBuf, 128, "<filename><modified?> - Supvime");
	SDL_SetWindowTitle(window, titleBuf);
}

