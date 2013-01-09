#ifndef traktor_render_ITextureBinding_H
#define traktor_render_ITextureBinding_H

#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ContextOpenGL;

class ITextureBinding
{
public:
#if defined(T_OPENGL_STD)
	virtual void bindSampler(ContextOpenGL* renderContext, GLuint unit, const GLuint sampler[], GLint locationTexture) = 0;
#else
	virtual void bindSampler(GLuint unit, const SamplerState& samplerState, GLint locationTexture) = 0;
#endif

	virtual void bindSize(GLint locationSize) = 0;
};
	
	}
}

#endif	// traktor_render_ITextureBinding_H
