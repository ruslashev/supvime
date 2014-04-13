#include "mesh.hpp"
#include "../glutils.hpp"

void Mesh::Create(float x, float y, float w, float h)
{
	const char *VertexShaderSrc = GLSL(
		attribute vec2 inVertCoord;
		void main() {
			gl_Position = vec4(inVertCoord, 0, 1);
		}
	);
	const char *FragmentShaderSrc = GLSL(
		void main() {
			gl_FragColor = vec4(1);
		}
	);
	vertShader = CreateShader(GL_VERTEX_SHADER, VertexShaderSrc);
	fragShader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSrc);
	shaderProg = CreateShaderProgram(vertShader, fragShader);
	vertCoordAttribute = BindAttribute(shaderProg, "inVertCoord");

	GLfloat vertices[] = {
		x,   y,
		x+w, y,
		x,   y+h,
		x+w, y+h
	};
	glGenBuffers(1, &vertCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(vertCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void Mesh::Draw()
{
	glUseProgram(shaderProg);
	glEnableVertexAttribArray(vertCoordAttribute);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDisableVertexAttribArray(vertCoordAttribute);
}

Mesh::~Mesh()
{
	glDeleteProgram(shaderProg);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);
}

