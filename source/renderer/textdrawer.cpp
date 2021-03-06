#include "textdrawer.hpp"
#include "../glutils.hpp"
#include "../errors.hpp"

TextDrawer::TextDrawer(const char *fontPath)
	: sx(2.f/800), sy(2.f/600), lineSpacing(1.35f)
{
	int errCode = FT_Init_FreeType(&ftLib);
	assertf(errCode == 0, "Failed to initialize FreeType");

	errCode = FT_New_Face(ftLib, fontPath, 0, &mainFace);
	assertf(errCode == 0, "Failed to create a new face for font \"%s\"", fontPath);

	if (!FT_IS_FIXED_WIDTH(mainFace))
		printf("Warning: Font face \"%s %s\" (%s) is not fixed width!\n",
				mainFace->family_name, mainFace->style_name, fontPath);

	InitGL();

	cacher.face = mainFace;
	cacher.ftLib = ftLib;
	cacher.ted = this;
	cacher.Precache(14);

	setGlobalTransformation(0, 0);
}

void TextDrawer::InitGL()
{
	GLenum err = glewInit();
	assertf(err == GLEW_OK, "Failed to initialize GLEW: %s", glewGetErrorString(err));
	assertf(GLEW_VERSION_2_1, "Your graphics card's OpenGL version is less than 2.1.");

	const char *ForegroundVertShaderSrc = GLSL(
		attribute vec2 inVertCoord;
		attribute vec2 inTextureCoord;
		varying vec2 outTextureCoord;

		uniform vec2 transformation;
		uniform vec2 globalTransformation;

		void main() {
			vec2 result = inVertCoord+transformation+globalTransformation;
			gl_Position = vec4(result, 0, 1);
			outTextureCoord = inTextureCoord;
		}
	);
	const char *ForegroundFragShaderSrc = GLSL(
		varying vec2 outTextureCoord;
		uniform sampler2D tex0;
		uniform vec3 fg;

		void main() {
			gl_FragColor = vec4(fg, texture2D(tex0, outTextureCoord).r);
		}
	);

	const char *BackgroundVertShaderSrc = GLSL(
		attribute vec2 inVertCoord;

		uniform vec2 transformation;
		uniform vec2 globalTransformation;

		void main() {
			vec2 result = inVertCoord+transformation+globalTransformation;
			gl_Position = vec4(result, 0, 1);
		}
	);
	const char *BacgroundFragShaderSrc = GLSL(
		uniform vec3 bg;

		void main() {
			gl_FragColor = vec4(bg, 1);
		}
	);

	fg_vertShader = CreateShader(GL_VERTEX_SHADER, ForegroundVertShaderSrc);
	fg_fragShader = CreateShader(GL_FRAGMENT_SHADER, ForegroundFragShaderSrc);
	fg_shaderProgram = CreateShaderProgram(fg_vertShader, fg_fragShader);

	fg_vertCoordAttribute = BindAttribute(fg_shaderProgram, "inVertCoord");
	fg_textureCoordAttribute = BindAttribute(fg_shaderProgram, "inTextureCoord");

	fg_textureUnif = BindUniform(fg_shaderProgram, "tex0");
	fg_FGcolorUnif = BindUniform(fg_shaderProgram, "fg");
	fg_transfUnif = BindUniform(fg_shaderProgram, "transformation");
	fg_gtransfUnif = BindUniform(fg_shaderProgram, "globalTransformation");


	bg_vertShader = CreateShader(GL_VERTEX_SHADER, BackgroundVertShaderSrc);
	bg_fragShader = CreateShader(GL_FRAGMENT_SHADER, BacgroundFragShaderSrc);
	bg_shaderProgram = CreateShaderProgram(bg_vertShader, bg_fragShader);

	bg_vertCoordAttribute = BindAttribute(bg_shaderProgram, "inVertCoord");

	bg_BGcolorUnif = BindUniform(bg_shaderProgram, "bg");
	bg_transfUnif = BindUniform(bg_shaderProgram, "transformation");
	bg_gtransfUnif = BindUniform(bg_shaderProgram, "globalTransformation");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void TextDrawer::RenderString(const std::string str, float &dx, const float dy)
{
	float sdx = dx;
	for (auto &ch : str)
		RenderChar(ch, sdx, dy);
	dx = sdx;
}

void TextDrawer::RenderChar(const uint32_t ch, float &dx, const float dy)
{
	const glyph glyph = cacher.Lookup(ch, 14);
	const float xadv = glyph.xAdvance*sx;
	GLfloat transformation[2] = { dx, dy };

	// -------------------- background -----
	glUseProgram(bg_shaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, cacher.bg_cellVertCoordsVBO);
	glEnableVertexAttribArray(bg_vertCoordAttribute);
	glVertexAttribPointer(bg_vertCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glUniform2fv(bg_transfUnif, 1, transformation);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(bg_vertCoordAttribute);

	// -------------------- foreground -----
	glUseProgram(fg_shaderProgram);

	glBindBuffer(GL_ARRAY_BUFFER, glyph.fg_glyphVertCoordsVBO);
	glEnableVertexAttribArray(fg_vertCoordAttribute);
	glVertexAttribPointer(fg_vertCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, cacher.fg_texCoordsVBO);
	glEnableVertexAttribArray(fg_textureCoordAttribute);
	glVertexAttribPointer(fg_textureCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindTexture(GL_TEXTURE_2D, glyph.textureID);

	glUniform2fv(fg_transfUnif, 1, transformation);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(fg_vertCoordAttribute);
	glDisableVertexAttribArray(fg_textureCoordAttribute);

	dx += xadv;
}

void TextDrawer::setTextForeground(unsigned char r, unsigned char g, unsigned char b)
{
	glUseProgram(fg_shaderProgram);
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(fg_FGcolorUnif, 1, color);
}

void TextDrawer::setTextBackground(unsigned char r, unsigned char g, unsigned char b)
{
	glUseProgram(bg_shaderProgram);
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(bg_BGcolorUnif, 1, color);
}

void TextDrawer::setGlobalTransformation(float x, float y)
{
	GLfloat transf[2] = { x, -y };

	glUseProgram(fg_shaderProgram);
	glUniform2fv(fg_gtransfUnif, 1, transf);

	glUseProgram(bg_shaderProgram);
	glUniform2fv(bg_gtransfUnif, 1, transf);
}

void TextDrawer::setTextSize(unsigned int size)
{
	fontHeight = size;
	FT_Set_Pixel_Sizes(mainFace, size, size);
}

TextDrawer::~TextDrawer()
{
	glDeleteProgram(fg_shaderProgram);
	glDeleteShader(fg_vertShader);
	glDeleteShader(fg_fragShader);

	glDeleteProgram(bg_shaderProgram);
	glDeleteShader(bg_vertShader);
	glDeleteShader(bg_fragShader);

	FT_Done_Face(mainFace);
	FT_Done_FreeType(ftLib);
}

void TextCacher::Precache(unsigned int size)
{
	ted->setTextSize(size);
	for (int i = 0; i < 256; i++)
		Lookup(i, size);

	const GLfloat texCoords[4][2] = {
		{ 0, 0 },
		{ 1, 0 },
		{ 0, 1 },
		{ 1, 1 }
	};
	glGenBuffers(1, &fg_texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, fg_texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

	const int errCode = FT_Load_Char(face, ' ', FT_LOAD_RENDER);
	assertf(errCode == 0, "Failed to render space character");
	const float xadv = (face->glyph->advance.x >> 6)*ted->sx;
	const float height = ted->fontHeight*ted->lineSpacing*ted->sy;
	GLfloat bgCellVertCoords[4][2] = {
		{ 0,      0 },
		{ 0+xadv, 0 },
		{ 0,      0-height },
		{ 0+xadv, 0-height }
	};
	glGenBuffers(1, &bg_cellVertCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bg_cellVertCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bgCellVertCoords), bgCellVertCoords,
			GL_STATIC_DRAW);
}

glyph TextCacher::Lookup(uint32_t ch, unsigned int size)
{
	const glyphKey key = { ch, size };
	const FT_GlyphSlot g = face->glyph;

	if (normalGlyphs.find(key) != normalGlyphs.end()) {
		return normalGlyphs.at(key);
	} else { // doesn't exist
		if (ch < 1)
			ch = 0xFFFD; // REPLACEMENT CHARACTER, looks like this: <?>
		const int errCode = FT_Load_Char(face, ch, FT_LOAD_RENDER);
		assertf(errCode == 0, "Failed to render char '%c'", ch);

		const float x2 = g->bitmap_left*ted->sx;
		const float y2 = g->bitmap_top*ted->sy - ted->fontHeight*ted->sy;
		const float w  = g->bitmap.width*ted->sx;
		const float h  = g->bitmap.rows*ted->sy;
		GLfloat fgVertCoords[4][2] = {
			{ x2,   y2   },
			{ x2+w, y2   },
			{ x2,   y2-h },
			{ x2+w, y2-h }
		};
		GLuint fg_glyphVertCoordsVBO;
		glGenBuffers(1, &fg_glyphVertCoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, fg_glyphVertCoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(fgVertCoords), fgVertCoords,
				GL_STATIC_DRAW);

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
				g->bitmap.width, g->bitmap.rows, 0,
				GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		const glyph value = {
			fg_glyphVertCoordsVBO,
			textureID,
			g->advance.x >> 6,
			g->bitmap_left,
			g->bitmap_top,
			g->bitmap.width,
			g->bitmap.rows
		};

		normalGlyphs[key] = value;

		return value;
	}
}

TextCacher::~TextCacher()
{
	for (auto it = normalGlyphs.begin(); it != normalGlyphs.end(); ++it) {
		glDeleteBuffers(1, &it->second.fg_glyphVertCoordsVBO);
		glDeleteTextures(1, &it->second.textureID);
	}
	glDeleteBuffers(1, &fg_texCoordsVBO);
	glDeleteBuffers(1, &bg_cellVertCoordsVBO);
}

