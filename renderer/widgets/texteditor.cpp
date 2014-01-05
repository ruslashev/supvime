#include "texteditor.hpp"

enum { BG, FG };
const struct { unsigned char r, g, b; } palette[2] = {
	{ 20,  20,  20  },
	{ 255, 255, 255 }
};

TextEditor::TextEditor(const char *fontPath, SDL_Rect npos, SDL_Window *nwp)
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throwf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));

	wp = nwp;
	pos = npos;

	if (FT_Init_FreeType(&ft))
		throwf("Failed to initialize FreeType\n");
	if (FT_New_Face(ft, fontPath, 0, &fontFace))
		throwf("Failed to find font \"%s\"\n", fontPath);
	FT_Set_Pixel_Sizes(fontFace, 0, 15);

	InitGL();
}

void TextEditor::InitGL()
{
	glGenBuffers(1, &textVBO);

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
		uniform vec4 fg;
		uniform vec4 bg;

		void main() {
			gl_FragColor = mix(bg, fg, texture2D(tex0, texcoord).a);
		}
	);
#undef GLSL
	vertShader = CreateShader(GL_VERTEX_SHADER, vertShaderSrc);
	fragShader = CreateShader(GL_FRAGMENT_SHADER, fragShaderSrc);

	shaderProgram = CreateShaderProgram(vertShader, fragShader);

	text_coordAttribute = glGetAttribLocation(shaderProgram, "coord");
	if (text_coordAttribute == -1)
		throwf("Failed to bind attribute \"coord\"\n");

	fontTextureUnif = BindUniform(shaderProgram, "tex0");
	fontFGcolorUnif = BindUniform(shaderProgram, "fg");
	fontBGcolorUnif = BindUniform(shaderProgram, "bg");
}

void TextEditor::Draw()
{
	glViewport(pos.x, pos.y, pos.w, pos.h);
	glUseProgram(shaderProgram);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// actual draw
	{
		GLfloat white[4] = {1, 1, 1, 1};
		GLfloat bg[4] = {0.1f, 0.1f, 0.1f, 1};
		glUniform4fv(fontFGcolorUnif, 1, white);
		glUniform4fv(fontBGcolorUnif, 1, bg);

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

void TextEditor::RenderText(const char *text, float x, float y, float sx, float sy)
{
	GLuint fontTexture;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUniform1i(fontTextureUnif, GL_TEXTURE0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glEnableVertexAttribArray(text_coordAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, textVBO);
	glVertexAttribPointer(text_coordAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

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

		GLfloat quad[4][4] = {
			{ x2,   -y2  , 0, 0 },
			{ x2+w, -y2  , 1, 0 },
			{ x2,   -y2-h, 0, 1 },
			{ x2+w, -y2-h, 1, 1 },
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (g->advance.x >> 6) * sx;
		y += (g->advance.y >> 6) * sy;
	}

	glDisableVertexAttribArray(text_coordAttribute);
	glDeleteTextures(1, &fontTexture);
}

// void TextEditor::markBlock(int sx, int sy, int ex, int ey)
// {
// 	for (int y = sy; y <= ey; y++) {
// 		// screen[y].dirty = true;
// 		for (int x = sx; x <= ex; x++)
// 			screen[y][x].flags |= 8; // inverse for now
// 	}
// }

GLuint TextEditor::CreateShader(GLenum type, const char *src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		PrintLog(shader);
		throwf("Failed to compile %s shader\n",
				type == GL_VERTEX_SHADER ? "vertex" : "fragment");
	}

	return shader;
}

GLuint TextEditor::CreateShaderProgram(GLuint vs, GLuint fs)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	GLint success = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		PrintLog(program);
		throwf("Failed to create shader program\n");
	}
	return program;
}

void TextEditor::PrintLog(GLuint &shaderOrProg)
{
	GLint logLen = 0;
	char *log;

	if (glIsShader(shaderOrProg)) {
		glGetShaderiv(shaderOrProg, GL_INFO_LOG_LENGTH, &logLen);
		log = new char [logLen];
		glGetShaderInfoLog(shaderOrProg, logLen, NULL, log);
	} else {
		glGetProgramiv(shaderOrProg, GL_INFO_LOG_LENGTH, &logLen);
		log = new char [logLen];
		glGetProgramInfoLog(shaderOrProg, logLen, NULL, log);
	}

	puts(log);
	delete [] log;
}

GLint TextEditor::BindUniform(GLuint shaderProgramP, const char *name)
{
	GLint unif = glGetUniformLocation(shaderProgramP, name);
	if (unif == -1)
		throwf("Failed to bind uniform \"%s\"\n", name);
	return unif;
}

TextEditor::~TextEditor()
{
	// if (texture)
	// 	SDL_DestroyTexture(texture);
	// glDisableVertexAttribArray(0);
	glDeleteBuffers(1, &textVBO);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
	glDeleteProgram(shaderProgram);
	FT_Done_FreeType(ft);
}

