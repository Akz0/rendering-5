#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include "Shader.h"
#include "stb/stb_image.h"	
#include "glm/glm.hpp"
#include <glm/gtc/type_ptr.hpp>
class Texture
{
public:
	GLuint ID;
	const char* type;
	const char* textureFile;
	GLuint unit;
	glm::vec2 resolution;

	void Initialize(const char* type, const char* textureFile);
	void Generate(GLuint slot,GLenum pixelType);
	
	void Load(Shader& shader, const char* uniform, GLuint unit);
	
	void Bind();
	void Unbind();
	void Delete();
};


#endif // !TEXTURE_CLASS_H