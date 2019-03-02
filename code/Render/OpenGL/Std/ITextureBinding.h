#pragma once

#include "Render/OpenGL/Std/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ITextureBinding
{
public:
	virtual void bindTexture(GLuint textureUnit) const = 0;

	virtual void bindImage(GLuint imageUnit) const = 0;

	virtual void bindSize(GLint locationSize) const = 0;

	virtual bool haveMips() const = 0;
};

	}
}
