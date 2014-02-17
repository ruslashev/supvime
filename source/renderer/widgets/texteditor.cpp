#include "texteditor.hpp"
#include "../../editor.hpp"
#include "../../glutils.hpp"
#include "../../errors.hpp"

TextEditor::TextEditor(const char *fontPath)
	: sx(2.f/800), sy(2.f/600), lineSpacing(1.35f)
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

	glGenBuffers(1, &textVBO_vertCoords);
	glGenBuffers(1, &textVBO_textureCoords);

	const char *VertShaderSrc = GLSL(
		attribute vec2 vertCoords;
		attribute vec2 textureCoords;

		varying vec2 textureCoordsP;

		void main() {
			gl_Position = vec4(vertCoords, 0, 1);
			textureCoordsP = textureCoords;
		}
	);

	const char *FragShaderSrc = GLSL(
		varying vec2 textureCoordsP;
		uniform sampler2D tex0;
		uniform vec3 fg;
		uniform vec3 bg;

		void main() {
			vec3 color = mix(bg, fg, texture2D(tex0, textureCoordsP).r);
			gl_FragColor = vec4(color, 1);
		}
	);

	text_vertShader = CreateShader(GL_VERTEX_SHADER, VertShaderSrc);
	text_fragShader = CreateShader(GL_FRAGMENT_SHADER, FragShaderSrc);
	text_shaderProgram = CreateShaderProgram(text_vertShader, text_fragShader);

	text_vertCoordsAttribute = BindAttribute(text_shaderProgram, "vertCoords");
	text_textureCoordsAttribute = BindAttribute(text_shaderProgram, "textureCoords");

	text_textureUnif = BindUniform(text_shaderProgram, "tex0");
	text_FGcolorUnif = BindUniform(text_shaderProgram, "fg");
	text_BGcolorUnif = BindUniform(text_shaderProgram, "bg");

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glUseProgram(text_shaderProgram);
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
	const float cellHeight = (int)(fontHeight*lineSpacing)*sy;

	for (size_t l = 0; l < ep->lines.size(); l++) {
		float dx = -1;
		const float dy = 1 - l*cellHeight;
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
				RenderChar('|', dx, dy, cx);
				cx++;
				for (int i = 1; i < spacesToInsert; i++) {
					RenderChar('-', dx, dy, cx);
					cx++;
				}
				cx--;
				setTextForeground(0, 0, 0);
			} else
				RenderChar(srcLine[c], dx, dy, cx);
		}
	}
}

void TextEditor::RenderChar(const uint32_t ch, float &dx, const float dy, const int cx)
{
	const glyph_t glyph = cacher.Lookup(ch, 14);
	const float xadv = glyph.xAdvance*sx;

	// Vertex coords
	GLfloat triStripVCoords[4][2] = {
		{ dx,      dy },
		{ dx+xadv, dy },
		{ dx,      dy+fontHeight*lineSpacing*sy },
		{ dx+xadv, dy+fontHeight*lineSpacing*sy }
	};

	glBindBuffer(GL_ARRAY_BUFFER, textVBO_vertCoords);
	glEnableVertexAttribArray(text_vertCoordsAttribute);
	glVertexAttribPointer(text_vertCoordsAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triStripVCoords), triStripVCoords,
			GL_DYNAMIC_DRAW); // TODO dynamic?

	// Texture coords
	const float x2 = dx + glyph.left;
	const float y2 = dy + glyph.top;
	const float w = glyph.width;
	const float h = glyph.height;

	GLfloat textureCoords[4][2] = {
		{ 0, 1 },
		{ 1, 1 },
		{ 0, 0 },
		{ 1, 0 }
	};

	glBindBuffer(GL_ARRAY_BUFFER, textVBO_textureCoords);
	glEnableVertexAttribArray(text_textureCoordsAttribute);
	glVertexAttribPointer(text_textureCoordsAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords,
			GL_DYNAMIC_DRAW); // TODO again

	glBindTexture(GL_TEXTURE_2D, glyph.textureID);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	dx = -1 + (cx+1)*xadv;
}

void TextEditor::setTextForeground(unsigned char r, unsigned char g, unsigned char b)
{
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(text_FGcolorUnif, 1, color);
}

void TextEditor::setTextBackground(unsigned char r, unsigned char g, unsigned char b)
{
	GLfloat color[3] = { r/255.f, g/255.f, b/255.f };
	glUniform3fv(text_BGcolorUnif, 1, color);
}

void TextEditor::setTextSize(unsigned int size)
{
	fontHeight = size;
	FT_Set_Pixel_Sizes(mainFace, size, size);
}

TextEditor::~TextEditor()
{
	glDeleteBuffers(1, &textVBO_vertCoords);
	glDeleteBuffers(1, &textVBO_textureCoords);
	glDeleteProgram(text_shaderProgram);
	glDeleteShader(text_vertShader);
	glDeleteShader(text_fragShader);

	FT_Done_Face(mainFace);
	FT_Done_FreeType(ftLib);
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

