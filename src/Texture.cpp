#include "Texture.h"

void Texture::Generate(GLuint slot, GLenum pixelType) {
	GLenum format = GL_RGB;
	int imageWidth, imageHeight, ColorChannels;
	stbi_set_flip_vertically_on_load(false);
	unsigned char* image_data = stbi_load(textureFile, &imageWidth, &imageHeight, &ColorChannels, 0);
	if (!image_data) {
		std::cerr << "CANNOT LOAD TEXTURE FROM :" << textureFile << std::endl;
	}

	if (ColorChannels == 4) {
		format = GL_RGBA;
	}
	else if (ColorChannels == 3) {
		format = GL_RGB;
	}
	else if (ColorChannels == 1) {
		format = GL_R;
	}

	resolution = glm::vec2(imageWidth, imageHeight);

	glGenTextures(1, &ID);
	glActiveTexture(GL_TEXTURE0 + slot);
	unit = slot;

	glBindTexture(GL_TEXTURE_2D, ID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, format, imageWidth, imageHeight, 0, format, pixelType, image_data);
	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(image_data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Initialize(const char* type, const char* textureFile){
	this->type = type;
	this->textureFile = textureFile;
};

void Texture::Load(Shader& shader, const char* uniform, GLuint unit)
{
	shader.Activate();
	GLuint texUni = glGetUniformLocation(shader.ID, uniform);
	glUniform2fv(glGetUniformLocation(shader.ID, "resolution"),1, glm::value_ptr(resolution));
	glUniform1i(texUni, unit);
}

void Texture::Bind()
{
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Delete()
{
	glDeleteTextures(1, &ID);
}