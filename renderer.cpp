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

	widgets.push_back(std::unique_ptr<TextEditor>(new TextEditor(80, 25, "DroidSansMono.ttf", 13, renderer)));
	widgets[0]->ep = ep;
}

void Renderer::UpdateTitle()
{
	char titleBuf[128];
	snprintf(titleBuf, 128, "%s <modified?> - Supvime", ep->fp->filename.c_str());
	SDL_SetWindowTitle(window, titleBuf);
}

void Renderer::Update(std::vector<std::string> &lines)
{
	SDL_RenderClear(renderer);

	widgets[0]->lines = lines;
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

TextEditor::TextEditor(int ncols, int nrows, const char *fontPath, int fontSize, SDL_Renderer *nrend)
{
	cols = ncols;
	rows = nrows;

	// Open font
	font = TTF_OpenFont(fontPath, fontSize);
	if (font == NULL) {
		printf("TTF error: %s\n", TTF_GetError());
		exit(2);
	}

	// Get glyph size
	fontHeight = TTF_FontHeight(font);
	TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &fontWidth);

	TTF_SetFontHinting(font, TTF_HINTING_LIGHT);

	screen.resize(rows);
	for (int y = 0; y < rows; y++)
		screen[y].resize(cols);

	rend = nrend;

	textAreaSurf = SDL_CreateRGBSurface(0, cols*fontWidth, rows*fontHeight, 32, 0, 0, 0, 0);
	if (textAreaSurf == NULL) {
		printf("failed to create surface: %s\n", SDL_GetError());
		exit(3);
	}
	textAreaTexture = SDL_CreateTexture(rend,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			cols*fontWidth, rows*fontHeight);
	if (textAreaTexture == NULL) {
		printf("failed to create texture: %s\n", SDL_GetError());
		exit(3);
	}
}

void TextEditor::RebuildSurface()
{
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			SDL_Color fg = { 255, 255, 255, 255 }, bg = { 20, 20, 20, 255 };
			const uint8_t flags = screen[y][x].flags;
			if (flags & 8) {
				const SDL_Color tempForSwap = fg;
				fg = bg;
				bg = tempForSwap;
			}
			SDL_Surface *cellSurf = TTF_RenderUTF8_Shaded(font, screen[y][x].ch.c_str(), fg, bg);
			SDL_Rect offsetRect = { fontWidth*x, fontHeight*y, 0, 0 };
			SDL_BlitSurface(cellSurf, NULL, textAreaSurf, &offsetRect);
			SDL_FreeSurface(cellSurf);
		}
	}
	SDL_UpdateTexture(textAreaTexture, NULL, textAreaSurf->pixels, textAreaSurf->pitch);
	const SDL_Rect textAreaRect = { 0, 0, cols*fontWidth, rows*fontHeight };
	SDL_RenderCopy(rend, textAreaTexture, NULL, &textAreaRect);
}

void TextEditor::Draw()
{
	clear();

	int i = 0;
	for (; i < (int)lines.size(); i++)
		mvaddstr(i, 0, lines[i]);
	for (; i <= rows-3; i++)
		mvaddstr(i, 0, "~");

	markBlock(ep->curs.x, ep->curs.y, ep->curs.x, ep->curs.y);

	RebuildSurface();
}

void TextEditor::move(int y, int x)
{
	drwCurs = { (unsigned int)x, (unsigned int)y };
}
void TextEditor::addch(std::string c)
{
	if ((int)drwCurs.x <= cols-1 && (int)drwCurs.y <= rows-1)
		screen[drwCurs.y][drwCurs.x].ch = c;
}
void TextEditor::addstr(std::string str)
{
	for (int x = drwCurs.x; x < (int)drwCurs.x+(int)str.length(); x++) {
		if (x >= cols)
			break;
		screen[drwCurs.y][x].ch = str[x-drwCurs.x];
	}
}
void TextEditor::mvaddch(int y, int x, std::string c)
{
	move(y, x);
	addch(c);
}
void TextEditor::mvaddstr(int y, int x, std::string str)
{
	move(y, x);
	addstr(str);
}

void TextEditor::clear()
{
	const Cell emptyCell = Cell { " ", 0, 0 };
	for (int y = 0; y < rows; y++)
		for (int x = 0; x < cols; x++)
			screen[y][x] = emptyCell;
}

void TextEditor::markBlock(int sx, int sy, int ex, int ey)
{
	for (int y = sy; y <= ey; y++)
		for (int x = sx; x <= ex; x++)
			screen[y][x].flags |= 8; // inverse for now
}

TextEditor::~TextEditor()
{
	SDL_FreeSurface(textAreaSurf);
	SDL_DestroyTexture(textAreaTexture);
	TTF_CloseFont(font);
}

