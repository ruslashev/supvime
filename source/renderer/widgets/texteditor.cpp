#include "texteditor.hpp"
#include "../../editor.hpp"
#include "../../glutils.hpp"
#include "../../errors.hpp"

const std::string Faces[] = {
	"symlink-to-font"
};

static FT_Error FtcFaceRequesterCb(
		FTC_FaceID faceID, FT_Library lib, FT_Pointer reqData, FT_Face *aface)
{
	const std::string faceName = Faces[(size_t)faceID];

	int errCode = FT_New_Face(lib, faceName.c_str(), (FT_Long)faceID, aface);
	assertf(errCode == 0, "Failed to load font face! Filename: \"%s\"", reqData);

	if (!((*aface)->face_flags & FT_FACE_FLAG_FIXED_WIDTH))
		printf("Warning: Font face \"%s %s\" (%s) is not fixed width!\n",
				(*aface)->family_name, (*aface)->style_name, faceName.c_str());

	return 0;
}

TextEditor::TextEditor(const char *fontPath)
	: sx(2.f/800), sy(2.f/600)
{
	int errCode = FT_Init_FreeType(&ftLib);
	assertf(errCode == 0, "Failed to initialize FreeType");

	errCode = FTC_Manager_New(ftLib, 1, 1, 2*1024*1024, // TODO: try 0 (default)
			&FtcFaceRequesterCb, NULL, &ftcManager);
	assertf(errCode == 0, "Failed to create Font cache manager!");
	errCode = FTC_CMapCache_New(ftcManager, &cmapCache);
	assertf(errCode == 0, "Failed to create Font CMap cache!");
	// if (FTC_SBitCache_New(ftcManager, &sbitCache))
	// 	throwf("Failed to create Font SBit cache!\n");

	errCode = FTC_ImageCache_New(ftcManager, &imgCache);
	assertf(errCode == 0, "Failed to create Font Image cache!");

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
	// setTextSize(14);
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

	// TODO temporary
	fontHeight = 14;
	const float vadv = fontHeight*sy;
	const float cellHeight = (int)(fontHeight*1.35f)*sy;

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

void TextEditor::RenderChar(const uint32_t ch, float &dx, const float dy, const float vadv, const int cx)
{
	FTC_ImageTypeRec glyphImageType;
	glyphImageType.face_id = 0;
	glyphImageType.width = 14;
	glyphImageType.height = 14;
	glyphImageType.flags = FT_LOAD_DEFAULT;

	FT_UInt glyphIndex = FTC_CMapCache_Lookup(cmapCache, /* TODO */ 0, 0, ch);
	assertf(glyphIndex != 0, "Failed to lookup glyph index for char '%c'", ch);
	FT_Glyph glyph;
	int errCode = FTC_ImageCache_Lookup(imgCache, &glyphImageType, ch, &glyph, NULL);
	assertf(errCode == 0, "Failed to lookup glyph image for char '%c'", ch);

	// FT_RENDER_MODE_LIGHT
	// FT_RENDER_MODE_LCD
	// FT_RENDER_MODE_LCD_V
	errCode = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, NULL, 0);
	assertf(errCode == 0, "Failed to render glyph '%c'", ch);

	assertf(glyph->format == FT_GLYPH_FORMAT_BITMAP,
			"Invalid glyph format: %d", glyph->format);

	FT_BitmapGlyph bitmapGl = (FT_BitmapGlyph)glyph;
	FT_Bitmap *sourceBitmap = &bitmapGl->bitmap;

	const int rows  = sourceBitmap->rows;
	const int width = sourceBitmap->width;
	const int left  = bitmapGl->left;
	const int top   = bitmapGl->top;

	const float xadv  = ((glyph->advance.x + 0x8000) >> 16 )*sx;
	const float yadv  = ((glyph->advance.y + 0x8000) >> 16 )*sx;

	const unsigned char *bitmapBuffer = sourceBitmap->buffer;
	// ----------------------------------------

	const float x2 = dx + left*sx;
	const float y2 = dy + top*sy;
	const float w  = width*sx;
	const float h  = rows*sy;

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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
			width, rows, 0,
			GL_RED, GL_UNSIGNED_BYTE, bitmapBuffer);

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

// void TextEditor::setTextSize(unsigned int size)
// {
// 	fontHeight = size;
// 	FT_Set_Pixel_Sizes(fontFace, size, size);
// }

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
	FTC_Manager_Done(ftcManager);
	FT_Done_FreeType(ftLib);
}

