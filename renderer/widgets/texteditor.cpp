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
	for (int y = 0; y < rows; y++) {
		screen[y].cells.resize(cols);
		// screen[y].dirty = true;
	}

	rend = nrend;
	ep = nep;

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
		puts("TextEditor::RebuildSurface() WARNING: no active texture!");
		return;
	}

	uint32_t* textPixels;
	int pitch;
	SDL_LockTexture(texture, NULL, (void**)&textPixels, &pitch);

	// puts("-- BEGIN draw");
	for (int y = 0; y < rows; y++) {
		SDL_Color fg = { 255, 255, 255, 255 }, bg = { 20, 20, 20, 255 };
		////
		std::string rowStr = "";
		for (int c = 0; c < cols; c++)
			rowStr += screen[y][c].ch;
		////
		SDL_Surface *rowSurf = TTF_RenderUTF8_Shaded(font, rowStr.c_str(), fg, bg);

		for (int x = 0; x < cols; x++) {
			const uint8_t flags = screen[y][x].flags;

			Uint8 r, g, b;
			for (int sry = 0; sry < fontHeight; sry++) {
				for (int srx = 0; srx < fontWidth; srx++) {
					const Uint32 px = getpixel(rowSurf, x*fontWidth+srx, sry);
					SDL_GetRGB(px, /* TODO */ rowSurf->format, &r, &g, &b);

					if (flags & 8) {
						r = 255-r;
						g = 255-g;
						b = 255-b;
					}

					const int i = (x*fontWidth)+srx + (((y*fontHeight)+sry) * fontWidth*cols);
					textPixels[i] = (r << 16) + (g << 8) + b;
				}
			}
		}
		SDL_FreeSurface(rowSurf);
	}
	// puts("-- END draw");

	SDL_UnlockTexture(texture);

	const SDL_Rect textAreaRect = { 0, 0, cols*fontWidth, rows*fontHeight };
	SDL_RenderCopy(rend, texture, NULL, &textAreaRect);

	for (int y = 0; y < rows; y++)
		screen[y].dirty = false;
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
	// screen[drwCurs.y].dirty = true;
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
	for (int y = sy; y <= ey; y++) {
		// screen[y].dirty = true;
		for (int x = sx; x <= ex; x++)
			screen[y][x].flags |= 8; // inverse for now
	}
}

TextEditor::~TextEditor()
{
	if (texture)
		SDL_DestroyTexture(texture);
	TTF_CloseFont(font);
}

