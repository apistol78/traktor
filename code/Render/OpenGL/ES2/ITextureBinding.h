#pragma once

#include "Render/OpenGL/ES2/TypesOpenGLES2.h"

namespace traktor
{
	namespace render
	{

class ITextureBinding
{
public:
	virtual void bindSampler(GLuint unit, const SamplerStateOpenGL& samplerState, GLint locationTexture) = 0;

	virtual void bindSize(GLint locationSize) = 0;
};

	}
}

