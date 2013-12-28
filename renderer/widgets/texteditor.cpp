#include "texteditor.hpp"

enum { BG, FG };
const struct { unsigned char r, g, b; } palette[2] = {
	{ 20,  20,  20  },
	{ 255, 255, 255 }
};

TextEditor::TextEditor(int ncols, int nrows, const char *fontPath, SDL_Rect npos, SDL_Window *nwp)
{
	cols = ncols;
	rows = nrows;
	screen.resize(rows);
	for (int y = 0; y < rows; y++)
		screen[y].cells.resize(cols);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		exit(1);
	}

	stash = glfonsCreate(512, 512, FONS_ZERO_TOPLEFT);
	if (stash == NULL) {
		puts("Failed to create font stash");
		exit(2);
	}

	font = fonsAddFont(stash, "sans", fontPath);
	if (font == FONS_INVALID) {
		printf("Failed to load font \"%s\"\n", fontPath);
		exit(2);
	}

	wp = nwp;
	pos = npos;
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
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 800, 600, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glColor4ub(255, 255, 255, 255);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);

	unsigned int fg = glfonsRGBA(255, 255, 255, 255);
	unsigned int bg = glfonsRGBA(20,  20,  20,  255);

	fonsClearState(stash);

	fonsSetSize(stash, 16.f);
	fonsSetFont(stash, font);
	fonsVertMetrics(stash, NULL, NULL, &fontHeight);
	float sx = pos.x, sy = pos.y+fontHeight;
	fonsSetColor(stash, fg);
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			const char *glyph;
			// if (screen[y][x].ch == "\t")
			// 	glyph = "    ";
			// else
				glyph = screen[y][x].ch.c_str();
			sx = fonsDrawText(stash, sx, sy, glyph, NULL);
		}
		sx = pos.x;
		sy += fontHeight;
	}

	glEnable(GL_DEPTH_TEST);

	SDL_GL_SwapWindow(wp);
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
	glfonsDelete(stash);
}

