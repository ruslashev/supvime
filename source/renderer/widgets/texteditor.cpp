#include "texteditor.hpp"
#include "../../editor.hpp"
#include "../../glutils.hpp"
#include "../../errors.hpp"

TextEditor::TextEditor(const char *fontPath)
	: sx(2.f/800), sy(2.f/600)
{
	int errCode = FT_Init_FreeType(&ftLib);
	assertf(errCode == 0, "Failed to initialize FreeType");

	errCode = FT_New_Face(ftLib, fontPath, 0, &mainFace);
	assertf(errCode == 0, "Failed to create a new face");

	if (!FT_IS_FIXED_WIDTH(mainFace))
		printf("Warning: Font face \"%s %s\" (%s) is not fixed width!\n",
				mainFace->family_name, mainFace->style_name, fontPath);

	cacher.face = mainFace;
	cacher.ftLib = ftLib;

	InitGL();
}

void TextEditor::InitGL()
{
	GLenum err = glewInit();
	assertf(err == GLEW_OK, "Failed to initialize GLEW: %s", glewGetErrorString(err));
	assertf(GLEW_VERSION_2_1, "Your graphics card's OpenGL version is less than 2.1.");

	glGenBuffers(1, &fg_textVBO);
	glGenBuffers(1, &bg_textVBO);

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

	fg_vertShader = CreateShader(GL_VERTEX_SHADER, ForegroundVertShaderSrc);
	fg_fragShader = CreateShader(GL_FRAGMENT_SHADER, ForegroundFragShaderSrc);
	fg_shaderProgram = CreateShaderProgram(fg_vertShader, fg_fragShader);
	fg_coordAttribute = BindAttribute(fg_shaderProgram, "coord");
	fg_textureUnif = BindUniform(fg_shaderProgram, "tex0");
	fg_FGcolorUnif = BindUniform(fg_shaderProgram, "fg");

	bg_vertShader = CreateShader(GL_VERTEX_SHADER, BackgroundVertShaderSrc);
	bg_fragShader = CreateShader(GL_FRAGMENT_SHADER, BacgroundFragShaderSrc);
	bg_shaderProgram = CreateShaderProgram(bg_vertShader, bg_fragShader);
	bg_vcoordAttribute = BindAttribute(bg_shaderProgram, "vcoord");
	bg_BGcolorUnif = BindUniform(bg_shaderProgram, "bg");
}

void TextEditor::Draw()
{
	setTextSize(14);
	setTextForeground(0, 0, 0);
	setTextBackground(255, 255, 255);

	RenderFile();

	for (size_t i = 0; i < ep->lines.size(); i++)
		ep->lines.at(i).dirty = false;
}

void TextEditor::RenderFile()
{
	glUseProgram(fg_shaderProgram);
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

	const float vadv = fontHeight*sy;
	const float cellHeight = (int)(fontHeight*1.35f)*sy; // TODO

	for (size_t l = 0; l < ep->lines.size(); l++) {
		float dx = -1;
		const float dy = 1 - l*cellHeight - vadv;
		const std::string srcLine = ep->lines.at(l).str;

		int cx = 0;
		for (size_t c = 0; c < srcLine.length(); c++, cx++) {
			if (l == ep->curs.y && c == ep->curs.x) {
				setTextBackground(100, 100, 100);
			} else {
				setTextBackground(255, 255, 255);
			}
			if (srcLine[c] == '\t') {
				const int tabsize = 4;
				const int spacesToInsert = tabsize - (cx % tabsize);
				setTextForeground(200, 200, 200);
				RenderChar('|', dx, dy, vadv, cx);
				cx++;
				for (int i = 1; i < spacesToInsert; i++) {
					RenderChar('-', dx, dy, vadv, cx);
					cx++;
				}
				cx--;
				setTextForeground(0, 0, 0);
			} else
				RenderChar(srcLine[c], dx, dy, vadv, cx);
		}
	}

	glDisableVertexAttribArray(bg_vcoordAttribute);
	glDisableVertexAttribArray(fg_coordAttribute);
	glDeleteTextures(1, &fontTexture);
}

glyph_t TextCacher::Lookup(uint32_t ch, unsigned int size)
{
	const glyphKey_t key = { ch, size };
	const FT_GlyphSlot g = face->glyph;

	if (normalGlyphs.find(key) != normalGlyphs.end()) {
		return normalGlyphs.at(key);
	} else {
		// doesn't exist

		const int errCode = FT_Load_Char(face, ch, FT_LOAD_RENDER);
		assertf(errCode == 0, "Failed to render char '%c'", ch);

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
				g->bitmap.width, g->bitmap.rows, 0,
				GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

		const glyph_t value = {
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
	for (auto it = normalGlyphs.begin(); it != normalGlyphs.end(); ++it)
		glDeleteTextures(1, &it->second.textureID);
}

void TextEditor::RenderChar(const uint32_t ch, float &dx, const float dy, const float yadv, const int cx)
{
	const glyph_t glyph = cacher.Lookup(ch, 14);
	const float xadv = glyph.xAdvance*sx;
	const float x2 = dx + glyph.left*sx;
	const float y2 = dy + glyph.top*sy;
	const float w = glyph.width*sx;
	const float h = glyph.height*sy;

	// -------------------- background -----
	GLfloat bgTriStrip[4][2] = {
		{ dx,      dy-yadv*0.35f },
		{ dx+xadv, dy-yadv*0.35f },
		{ dx,      dy+yadv },
		{ dx+xadv, dy+yadv },
	};

	glUseProgram(bg_shaderProgram);
	glEnableVertexAttribArray(bg_vcoordAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, bg_textVBO);
	glVertexAttribPointer(bg_vcoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bgTriStrip), bgTriStrip, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// -------------------- foreground -----
	glUseProgram(fg_shaderProgram);

	glEnableVertexAttribArray(fg_coordAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, fg_textVBO);
	glVertexAttribPointer(fg_coordAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glyph.textureID);

	GLfloat fgTriStrip[4][4] = {
		{ x2,   y2  , 0, 0 },
		{ x2+w, y2  , 1, 0 },
		{ x2,   y2-h, 0, 1 },
		{ x2+w, y2-h, 1, 1 },
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(fgTriStrip), fgTriStrip, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	dx = -1 + (cx+1)*xadv;
}

void TextEditor::setTextForeground(unsigned char r, unsigned char g, unsigned char b)
{
	glUseProgram(fg_shaderProgram);
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(fg_FGcolorUnif, 1, color);
}

void TextEditor::setTextBackground(unsigned char r, unsigned char g, unsigned char b)
{
	glUseProgram(bg_shaderProgram);
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(bg_BGcolorUnif, 1, color);
}

void TextEditor::setTextSize(unsigned int size)
{
	fontHeight = size;
	FT_Set_Pixel_Sizes(mainFace, size, size);
}

TextEditor::~TextEditor()
{
	glDeleteBuffers(1, &fg_textVBO);
	glDeleteProgram(fg_shaderProgram);
	glDeleteShader(fg_vertShader);
	glDeleteShader(fg_fragShader);
	glDeleteBuffers(1, &bg_textVBO);
	glDeleteProgram(bg_shaderProgram);
	glDeleteShader(bg_vertShader);
	glDeleteShader(bg_fragShader);
	FT_Done_Face(mainFace);
	FT_Done_FreeType(ftLib);
}

