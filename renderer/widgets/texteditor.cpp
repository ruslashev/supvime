#include "texteditor.hpp"

TextEditor::TextEditor(const char *fontPath, SDL_Rect npos, SDL_Window *nwp)
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throwf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
	if (!GLEW_VERSION_2_1)
		throwf("Your graphics card's OpenGL version is less than 2.1\n");

	wp = nwp;
	pos = npos;

	if (FT_Init_FreeType(&ft))
		throwf("Failed to initialize FreeType\n");
	if (FT_New_Face(ft, fontPath, 0, &fontFace))
		throwf("Failed to find font \"%s\"\n", fontPath);
	if (!FT_IS_FIXED_WIDTH(fontFace))
		printf("Warning: Font face \"%s %s\" (%s) is not fixed width!\n",
				fontFace->family_name, fontFace->style_name, fontPath);
	// if (!FT_HAS_VERTICAL(fontFace))
	// 	printf("Warning: Font face \"%s %s\" (%s) does not have vertical metrics!\n",
	// 			fontFace->family_name, fontFace->style_name, fontPath);

	InitGL();
}

void TextEditor::InitGL()
{
	glGenBuffers(1, &fg_textVBO);
	glGenBuffers(1, &bg_textVBO);

#define GLSL(src) "#version 120\n" #src
	const char *ForegroundVertShaderSrc = GLSL(
		attribute vec4 coord;
		varying vec2 texCoord;
		void main() {
			gl_Position = vec4(coord.xy, 0, 1);
			texCoord = coord.zw;
		}
	);

	const char *ForegroundFragShaderSrc = GLSL(
		varying vec2 texCoord;
		uniform sampler2D tex0;
		uniform vec3 fg;
		void main() {
			gl_FragColor = vec4(fg, texture2D(tex0, texCoord).r);
		}
	);

	const char *BackgroundVertShaderSrc = GLSL(
		attribute vec2 vcoord;
		void main() {
			gl_Position = vec4(vcoord, 0, 1);
		}
	);

	const char *BacgroundFragShaderSrc = GLSL(
		uniform vec3 bg;
		void main() {
			gl_FragColor = vec4(bg, 1);
		}
	);
#undef GLSL
	fgVertShader = CreateShader(GL_VERTEX_SHADER, ForegroundVertShaderSrc);
	fgFragShader = CreateShader(GL_FRAGMENT_SHADER, ForegroundFragShaderSrc);
	fgShaderProgram = CreateShaderProgram(fgVertShader, fgFragShader);
	fg_coordAttribute = BindAttribute(fgShaderProgram, "coord");
	fg_textureUnif = BindUniform(fgShaderProgram, "tex0");
	fg_FGcolorUnif = BindUniform(fgShaderProgram, "fg");

	bgVertShader = CreateShader(GL_VERTEX_SHADER, BackgroundVertShaderSrc);
	bgFragShader = CreateShader(GL_FRAGMENT_SHADER, BacgroundFragShaderSrc);
	bgShaderProgram = CreateShaderProgram(bgVertShader, bgFragShader);
	bg_vcoordAttribute = BindAttribute(bgShaderProgram, "vcoord");
	bg_BGcolorUnif = BindUniform(bgShaderProgram, "bg");
}

void TextEditor::Draw()
{
	glViewport(0, 0, 800, 600);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// actual draw
	{
		setTextSize(46);
		setTextForeground(0, 0, 0);
		setTextBackground(255, 255, 255);

		float sx = 2.0 / 800;
		float sy = 2.0 / 600;

		RenderText("T", //he Quick, \"Brown\" Fox Jumps Over The Lazy Dog.",
				-1 + 8 * sx,   1 - 50 * sy,    sx, sy);

		RenderText("The Quick, \"Brown\" Fox Jumps Over The Lazy Dog.",
				-1 + 8 * sx,   1 - 200 * sy,    sx, sy);

		setTextSize(15);
		RenderText(lines->at(0).str.c_str(),
				-1 + 40 * sx,   1 - 400 * sy,    sx, sy);
	}

	SDL_GL_SwapWindow(wp);

	for (size_t i = 0; i < lines->size(); i++)
		lines->at(i).dirty = false;
}

void TextEditor::RenderText(const char *text, float x, float y, float sx, float sy)
{
	glUseProgram(fgShaderProgram);
	glActiveTexture(GL_TEXTURE0);
	GLuint fontTexture;
	glGenTextures(1, &fontTexture);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUniform1i(fg_textureUnif, GL_TEXTURE0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	const FT_GlyphSlot g = fontFace->glyph;
	if (FT_Load_Char(fontFace, 'A', FT_LOAD_RENDER))
		throwf("failed to load char\n");
	const float adv = (g->advance.x >> 6)*sx;
	if (FT_Load_Char(fontFace, 'A', FT_LOAD_RENDER | FT_LOAD_VERTICAL_LAYOUT))
		throwf("failed to load char\n");
	const float vadv = (g->advance.y >> 6)*sy;
	printf("horz: %f\nvert: %f\ncell: %u\nclls: %f\n\n", adv, vadv, cellHeight, cellHeight*sy);

	for (const char *p = text; *p != '\0'; p++) {
		if (FT_Load_Char(fontFace, *p, FT_LOAD_RENDER))
			continue;

		const float x2 = x + g->bitmap_left*sx;
		const float y2 = y + g->bitmap_top*sy;
		const float w  = g->bitmap.width*sx;
		const float h  = g->bitmap.rows*sy;

		GLfloat bgQuad[4][2] = {
			{ x,     y-vadv*0.35f }, // fed up
			{ x+adv, y-vadv*0.35f },
			{ x,     y+vadv },
			{ x+adv, y+vadv },
		};

		glUseProgram(bgShaderProgram);
		glEnableVertexAttribArray(bg_vcoordAttribute);
		glBindBuffer(GL_ARRAY_BUFFER, bg_textVBO);
		glVertexAttribPointer(bg_vcoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBufferData(GL_ARRAY_BUFFER, sizeof(bgQuad), bgQuad, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		// -------------------- foreground -----
		glUseProgram(fgShaderProgram);

		glEnableVertexAttribArray(fg_coordAttribute);
		glBindBuffer(GL_ARRAY_BUFFER, fg_textVBO);
		glVertexAttribPointer(fg_coordAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
				g->bitmap.width, g->bitmap.rows, 0,
				GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		GLfloat fgQuad[4][4] = {
			{ x2,   y2  , 0, 0 },
			{ x2+w, y2  , 1, 0 },
			{ x2,   y2-h, 0, 1 },
			{ x2+w, y2-h, 1, 1 },
		};

		glBufferData(GL_ARRAY_BUFFER, sizeof(fgQuad), fgQuad, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		printf("%c", *p);
		x += adv;
	}
	printf("\n");

	glDisableVertexAttribArray(bg_vcoordAttribute);
	glDisableVertexAttribArray(fg_coordAttribute);
	glDeleteTextures(1, &fontTexture);
}

void TextEditor::setTextForeground(unsigned char r, unsigned char g, unsigned char b)
{
	glUseProgram(fgShaderProgram);
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(fg_FGcolorUnif, 1, color);
}

void TextEditor::setTextBackground(unsigned char r, unsigned char g, unsigned char b)
{
	glUseProgram(bgShaderProgram);
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(bg_BGcolorUnif, 1, color);
}

void TextEditor::setTextSize(unsigned int size)
{
	cellHeight = size;
	FT_Set_Pixel_Sizes(fontFace, size, size);
}

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
	GLint success;
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

GLint TextEditor::BindAttribute(GLuint shaderProgramP, const char *name)
{
	GLint attrib = glGetAttribLocation(shaderProgramP, name);
	if (attrib == -1)
		throwf("Failed to bind attribute \"%s\"\n", name);
	return attrib;
}

TextEditor::~TextEditor()
{
	glDeleteBuffers(1, &fg_textVBO);
	glDeleteProgram(fgShaderProgram);
	glDeleteShader(fgVertShader);
	glDeleteShader(fgFragShader);
	glDeleteBuffers(1, &bg_textVBO);
	glDeleteProgram(bgShaderProgram);
	glDeleteShader(bgVertShader);
	glDeleteShader(bgFragShader);
	FT_Done_Face(fontFace);
	FT_Done_FreeType(ft);
}

