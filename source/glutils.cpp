#include "glutils.hpp"
#include "errors.hpp"

GLuint CreateShader(GLenum type, const char *src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		PrintLog(shader);
		throwf("Failed to compile %s shader",
				type == GL_VERTEX_SHADER ? "vertex" : "fragment");
	}

	return shader;
}

GLuint CreateShaderProgram(GLuint vs, GLuint fs)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		PrintLog(program);
		throwf("Failed to create shader program");
	}
	return program;
}

void PrintLog(GLuint shaderOrProg)
{
	char log[1024];

	if (glIsShader(shaderOrProg))
		glGetShaderInfoLog(shaderOrProg, 1024, NULL, log);
	else
		glGetProgramInfoLog(shaderOrProg, 1024, NULL, log);

	printf("%s", log);
}

GLint BindUniform(GLuint shaderProgramP, const char *name)
{
	GLint unif = glGetUniformLocation(shaderProgramP, name);
	assertf(unif != -1, "Failed to bind uniform \"%s\"", name);
	return unif;
}

GLint BindAttribute(GLuint shaderProgramP, const char *name)
{
	GLint attrib = glGetAttribLocation(shaderProgramP, name);
	assertf(attrib != -1, "Failed to bind attribute \"%s\"", name);
	return attrib;
}

