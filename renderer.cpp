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
	fontHeight = TTF_FontHeight(font);
	TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &fontWidth);

	TTF_SetFontHinting(font, TTF_HINTING_LIGHT);
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

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

	screen.resize(rows);
	for (int y = 0; y < rows; y++)
		screen[y].resize(cols);
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
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			SDL_Color fg = {255, 255, 255, 255}, bg = {20, 20, 20, 255};
			const uint8_t flags = screen[y][x].flags;
			if (flags & 8) {
				const SDL_Color tempForSwap = fg;
				fg = bg;
				bg = tempForSwap;
			}
			SDL_Surface *cellSurf = TTF_RenderUTF8_Shaded(font, screen[y][x].ch.c_str(), fg, bg);
			SDL_Texture *cellTexture = SDL_CreateTextureFromSurface(renderer, cellSurf);
			SDL_Rect offsetRect = { cellSurf->w*x, cellSurf->h*y, cellSurf->w, cellSurf->h };
			SDL_RenderCopy(renderer, cellTexture, NULL, &offsetRect);
			SDL_FreeSurface(cellSurf);
			SDL_DestroyTexture(cellTexture);
		}
	}
}

void Renderer::Update(std::vector<std::string> &lines)
{
	clear();

	int i = 0;
	for (; i < (int)lines.size(); i++)
		mvaddstr(i, 0, lines[i]);
	for (; i <= rows-3; i++)
		mvaddstr(i, 0, "~");

	mvaddstr(rows-2, 0, ep->mode == 0 ? " NORMAL" : " INSERT");
	markBlock(0, rows-2, cols-1, rows-2);

	markBlock(ep->curs.x, ep->curs.y, ep->curs.x, ep->curs.y);

	RebuildSurface();

	SDL_RenderPresent(renderer);

	UpdateTitle();
}

void Renderer::clear()
{
	for (int y = 0; y < rows; y++)
		for (int x = 0; x < cols; x++)
			screen[y][x] = Cell { " ", 0, 0 };
}
void Renderer::move(int y, int x)
{
	drwCurs = { (unsigned int)x, (unsigned int)y };
}
void Renderer::addch(std::string c)
{
	if ((int)drwCurs.x <= cols-1 && (int)drwCurs.y <= rows-1)
		screen[drwCurs.y][drwCurs.x].ch = c;
}
void Renderer::addstr(std::string str)
{
	for (int x = drwCurs.x; x < (int)drwCurs.x+(int)str.length(); x++) {
		if (x >= cols)
			break;
		screen[drwCurs.y][x].ch = str[x-drwCurs.x];
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

void Renderer::markBlock(int sx, int sy, int ex, int ey)
{
	for (int y = sy; y <= ey; y++)
		for (int x = sx; x <= ex; x++)
			screen[y][x].flags |= 8; // inverse for now
}

void Renderer::UpdateTitle()
{
	char titleBuf[128];
	snprintf(titleBuf, 128, "%s <modified?> - Supvime", ep->fp->filename.c_str());
	SDL_SetWindowTitle(window, titleBuf);
}

