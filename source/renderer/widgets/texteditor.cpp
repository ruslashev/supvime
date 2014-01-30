#include "texteditor.hpp"
#include "../../editor.hpp"
#include "../../glutils.hpp"
#include "../../errors.hpp"

TextEditor::TextEditor(const char *fontPath) : sx(2.f/800), sy(2.f/600)
{
	GLenum err = glewInit();
	if (err != GLEW_OK)
		throwf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
	if (!GLEW_VERSION_2_1)
		throwf("Your graphics card's OpenGL version is less than 2.1\n");

	if (FT_Init_FreeType(&ft))
		throwf("Failed to initialize FreeType\n");
	if (FT_New_Face(ft, fontPath, 0, &fontFace))
		throwf("Failed to find font \"%s\"\n", fontPath);
	if (!FT_IS_FIXED_WIDTH(fontFace))
		printf("Warning: Font face \"%s %s\" (%s) is not fixed width!\n",
				fontFace->family_name, fontFace->style_name, fontPath);

	InitGL();
	g = fontFace->glyph;
}

void TextEditor::InitGL()
{
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

	if (FT_Load_Char(fontFace, 'A', 0))
		throwf("Failed to render primitive glyph!\n");

	const float vadv = fontHeight*sy;
	const float cellHeight = (int)(fontHeight*1.35f)*sy;
	const float adv = (fontFace->glyph->advance.x >> 6)*sx;

	for (size_t l = 0; l < ep->lines.size(); l++) {
		float dx = -1;
		const float dy = 1 - l*cellHeight - vadv;
		std::string srcLine = ep->lines.at(l).str;
		const size_t lineLen = srcLine.length();

		int cx = 0;
		for (size_t c = 0; c < lineLen; c++, cx++) {
			if (l == ep->curs.y && c == ep->curs.x) {
				setTextBackground(100, 100, 100);
			} else {
				setTextBackground(255, 255, 255);
			}
			if (srcLine[c] == '\t') {
				const int tabsize = 4;
				const int spacesToInsert = tabsize - (cx % tabsize);
				setTextForeground(200, 200, 200);
				RenderChar('|', dx, dy, adv, vadv, cx);
				cx++;
				for (int i = 1; i < spacesToInsert; i++) {
					RenderChar('-', dx, dy, adv, vadv, cx);
					cx++;
				}
				cx--;
				setTextForeground(0, 0, 0);
			}
			RenderChar(srcLine[c], dx, dy, adv, vadv, cx);
		}
	}

	glDisableVertexAttribArray(bg_vcoordAttribute);
	glDisableVertexAttribArray(fg_coordAttribute);
	glDeleteTextures(1, &fontTexture);
}

void TextEditor::RenderChar(const uint32_t ch, float &dx, const float dy, const float adv, const float vadv, const int cx)
{
	if (FT_Load_Char(fontFace, ch, FT_LOAD_RENDER) != 0)
		return;

	const float x2 = dx + g->bitmap_left*sx;
	const float y2 = dy + g->bitmap_top*sy;
	const float w  = g->bitmap.width*sx;
	const float h  = g->bitmap.rows*sy;

	GLfloat bgTriStrip[4][2] = {
		{ dx,     dy-vadv*0.35f },
		{ dx+adv, dy-vadv*0.35f },
		{ dx,     dy+vadv },
		{ dx+adv, dy+vadv },
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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
			g->bitmap.width, g->bitmap.rows, 0,
			GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);

	GLfloat fgTriStrip[4][4] = {
		{ x2,   y2  , 0, 0 },
		{ x2+w, y2  , 1, 0 },
		{ x2,   y2-h, 0, 1 },
		{ x2+w, y2-h, 1, 1 },
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(fgTriStrip), fgTriStrip, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	dx = -1 + (cx+1)*adv;
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
	FT_Set_Pixel_Sizes(fontFace, size, size);
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
	FT_Done_Face(fontFace);
	FT_Done_FreeType(ft);
}

