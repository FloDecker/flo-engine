#include "texture.h"

#include <GL/glew.h>

void texture::use(unsigned int textureUnit) const
{
	if (!initialized_) return;
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, texture_);
	glBindSampler(GL_TEXTURE0 + textureUnit, textureUnit);
}
