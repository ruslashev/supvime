#include "texteditor.hpp"

TextEditor::TextEditor(int ncols, int nrows, const char *fontPath, int fontSize, SDL_Renderer *nrend, Editor *nep)
{
	cols = ncols;
	rows = nrows;

	// Open font
	font = TTF_OpenFont(fontPath, fontSize);
	if (font == NULL) {
		printf("TTF error: %s\n", TTF_GetError());
		exit(2);
	}
	if (!TTF_FontFaceIsFixedWidth(font))
		puts("Warning: using a non monospace font");

	// Get glyph size
	fontHeight = TTF_FontHeight(font);
	TTF_GlyphMetrics(font, 'A', NULL, NULL, NULL, NULL, &fontWidth);

	screen.resize(rows);
	for (int y = 0; y < rows; y++)
		screen[y].resize(cols);

	rend = nrend;
	ep = nep;

	textAreaSurf = SDL_CreateRGBSurface(0, cols*fontWidth, rows*fontHeight, 32, 0, 0, 0, 0);
	if (textAreaSurf == NULL) {
		printf("failed to create surface: %s\n", SDL_GetError());
		exit(3);
	}
	texture = SDL_CreateTexture(rend,
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING,
			cols*fontWidth, rows*fontHeight);
	if (texture == NULL) {
		printf("failed to create texture: %s\n", SDL_GetError());
		exit(3);
	}
}

Uint32 getpixel(SDL_Surface *surface, const int x, const int y)
{
	const int bpp = surface->format->BytesPerPixel;
	return *((Uint8*)surface->pixels + y*surface->pitch + x*bpp);
}

void TextEditor::RebuildSurface()
{
	if (!texture) {
		puts("TextEditor::RebuildSurface() ERROR: no active texture!");
		return;
	}

	uint32_t* textPixels;
	int pitch;
	SDL_LockTexture(texture, NULL, (void**)&textPixels, &pitch);

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

			Uint8 r, g, b;
			for (int sry = 0; sry < fontHeight; sry++) {
				for (int srx = 0; srx < fontWidth; srx++) {
					const Uint32 px = getpixel(cellSurf, srx, sry);
					SDL_GetRGB(px, cellSurf->format, &r, &g, &b);
					const int i = (x*fontWidth)+srx + (((y*fontHeight)+sry) * fontWidth*cols);
					textPixels[i] = (r << 16) + (g << 8) + b;
				}
			}
			SDL_FreeSurface(cellSurf);
		}

		// SDL_Rect offsetRect = { 0, fontHeight*y, 0, 0 };
		// SDL_BlitSurface(cellSurf, NULL, textAreaSurf, &offsetRect);
	}

	SDL_UnlockTexture(texture);

	// SDL_UpdateTexture(textAreaTexture, NULL, textAreaSurf->pixels, textAreaSurf->pitch);
	const SDL_Rect textAreaRect = { 0, 0, cols*fontWidth, rows*fontHeight };
	SDL_RenderCopy(rend, texture, NULL, &textAreaRect);
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
	if (texture)
		SDL_DestroyTexture(texture);
	TTF_CloseFont(font);
}

