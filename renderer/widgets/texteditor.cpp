#include "texteditor.hpp"

TextEditor::TextEditor(int ncols, int nrows, const char *fontPath, int fontSize, SDL_Window *nwp)
{
	cols = ncols;
	rows = nrows;
	screen.resize(rows);
	for (int y = 0; y < rows; y++)
		screen[y].cells.resize(cols);

	stash = sth_create(512, 512);
	if (!stash) {
		puts("Failed to create font stash");
		exit(2);
	}

	std::ifstream fontIfs(fontPath, std::ios::in | std::ios::binary | std::ios::ate);
	if (!fontIfs) {
		printf("Failed to open font file \"%s\"\n", fontPath);
		exit(2);
	}
	std::streampos fontFileSize = fontIfs.tellg();
	fontData = new unsigned char[fontFileSize];
	fontIfs.seekg(0, std::ios::beg);
	fontIfs.read((char*)(&fontData[0]), fontFileSize);
	fontIfs.close();

	font = sth_add_font_from_memory(stash, fontData);
	if (!font) {
		printf("Failed to load font \"%s\"\n", fontPath);
		exit(2);
	}

	wp = nwp;
}

void TextEditor::RebuildSurface()
{
	SDL_Rect textPosRect = { 0, 0, cols*fontWidth, rows*fontHeight };

	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			SDL_Color fg = { 255, 255, 255, 255 }, bg = { 20, 20, 20, 255 };
		}
	}

	// SDL_UnlockTexture(texture);

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

	// markBlock(rp->ep->curs.x, ep->curs.y, ep->curs.x, ep->curs.y);

	glViewport(0, 0, 800, 600);
	glClearColor(0.1f, 0.1f, 0.1f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 800, 0, 600, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glColor4ub(255, 255, 255, 255);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sth_begin_draw(stash);
	float sx = 100, sy = 200;
	sth_draw_text(stash, font, 24.f, sx, sy, "Sup, world", &sx);
	sth_end_draw(stash);

	glEnable(GL_DEPTH_TEST);

	SDL_GL_SwapWindow(wp);

	// RebuildSurface();
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
	// if (texture)
	// 	SDL_DestroyTexture(texture);
	sth_delete(stash);
	delete [] fontData;
}

