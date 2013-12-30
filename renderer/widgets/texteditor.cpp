#include "texteditor.hpp"

enum { BG, FG };
const struct { unsigned char r, g, b; } palette[2] = {
	{ 20,  20,  20  },
	{ 255, 255, 255 }
};

TextEditor::TextEditor(const char *fontPath, SDL_Rect npos, SDL_Window *nwp)
{
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		exit(1);
	}

	wp = nwp;
	pos = npos;

	if (FT_Init_FreeType(&ft)) {
		puts("Failed to initialize FreeType");
		exit(1);
	}
	if (FT_New_Face(ft, fontPath, 0, &fontFace)) {
		printf("Failed to open font \"%s\"\n", fontPath);
		exit(2);
	}
	FT_Set_Pixel_Sizes(fontFace, 0, 15);

	InitGL();
}

void TextEditor::InitGL()
{
#define GLSL(src) "#version 120\n" #src
	// TODO 2 attributes instead of 1
	const char *vertShaderSrc = GLSL(
		attribute vec4 coord;
		varying vec2 texcoord;

		void main() {
			gl_Position = vec4(coord.xy, 0, 1);
			texcoord = coord.zw;
		}
	);
	const char *fragShaderSrc = GLSL(
		varying vec2 texcoord;
		uniform sampler2D tex0;
		uniform vec4 color;

		void main() {
			gl_FragColor = vec4(1, 1, 1, texture2D(tex0, texcoord).a) * color;
		}
	);
#undef GLSL
	GLint success = GL_FALSE;
	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertShaderSrc, NULL);
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		puts("Failed to compile vertex shader");
		GLint logLen = 0;
		glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &logLen);
		// not catching because will catch in main() in future
		char *log = new char [logLen];
		glGetShaderInfoLog(vertShader, logLen, NULL, log);
		puts(log);
		delete [] log;
		exit(2);
	}
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragShaderSrc, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		puts("Failed to compile fragment shader");
		GLint logLen = 0;
		glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLen);
		char *log = new char [logLen];
		glGetShaderInfoLog(fragShader, logLen, NULL, log);
		puts(log);
		delete [] log;
		exit(2);
	}

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		puts("Failed to create shader program");
		exit(2);
	}

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	fontTextureUnif = glGetUniformLocation(shaderProgram, "tex0");
	if (fontTextureUnif == -1) {
		printf("Failed to bind uniform \"tex0\"\n");
		exit(2);
	}
	glUniform1i(fontTextureUnif, GL_TEXTURE0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenBuffers(1, &fontVBO);
	glEnableVertexAttribArray(font_vcoordAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
	glVertexAttribPointer(font_vcoordAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

	fontColorUnif = glGetUniformLocation(shaderProgram, "color");
	if (fontColorUnif == -1) {
		printf("Failed to bind uniform \"color\"\n");
		exit(2);
	}
}

void TextEditor::RenderText(const char *text, float x, float y, float sx, float sy)
{
	const char *p;
	FT_GlyphSlot g = fontFace->glyph;

	for (p = text; *p; p++) {
		if (FT_Load_Char(fontFace, *p, FT_LOAD_RENDER))
			continue;

		glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_ALPHA,
				g->bitmap.width,
				g->bitmap.rows,
				0,
				GL_ALPHA,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer
				);

		float x2 = x + g->bitmap_left * sx;
		float y2 = -y - g->bitmap_top * sy;
		float w = g->bitmap.width * sx;
		float h = g->bitmap.rows * sy;

		GLfloat box[4][4] = {
			{x2,   -y2  , 0, 0},
			{x2+w, -y2  , 1, 0},
			{x2,   -y2-h, 0, 1},
			{x2+w, -y2-h, 1, 1},
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(box), box, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}
}

void TextEditor::Draw()
{
	glViewport(0, 0, 800, 600);
	glUseProgram(shaderProgram);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// actual draw
	{
		GLfloat fg[4] = {1, 1, 1, 1};
		glUniform4fv(fontColorUnif, 1, fg);

		float sx = 2.0 / 800;
		float sy = 2.0 / 600;

		RenderText("The Quick Brown Fox Jumps Over The Lazy Dog",
				-1 + 8 * sx,   1 - 50 * sy,    sx, sy);
		// RenderText("В чащах юга жил бы цитрус? Да, но фальшивый экземпляр!",
		// 		-1 + 8 * sx,   1 - 100 * sy,    sx, sy);
	}

	SDL_GL_SwapWindow(wp);

	for (size_t i = 0; i < lines->size(); i++)
		lines->at(i).dirty = false;
}

// void TextEditor::markBlock(int sx, int sy, int ex, int ey)
// {
// 	for (int y = sy; y <= ey; y++) {
// 		// screen[y].dirty = true;
// 		for (int x = sx; x <= ex; x++)
// 			screen[y][x].flags |= 8; // inverse for now
// 	}
// }

TextEditor::~TextEditor()
{
	// if (texture)
	// 	SDL_DestroyTexture(texture);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	glDeleteProgram(shaderProgram);
}

