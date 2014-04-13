#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>

class Mesh
{
	GLuint vertCoordsVBO;
	GLuint vertShader, fragShader, shaderProg;
	GLuint vertCoordAttribute;
public:
	void Create(float x, float y, float w, float h);
	void Draw();
	~Mesh();
};

#endif


