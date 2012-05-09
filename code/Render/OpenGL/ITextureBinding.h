#ifndef traktor_render_ITextureBinding_H
#define traktor_render_ITextureBinding_H

#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class ITextureBinding
{
public:
	virtual void bindSampler(GLuint unit, const SamplerState& samplerState, GLint locationTexture) = 0;

	virtual void bindSize(GLint locationSize) = 0;
};
	
	}
}

#endif	// traktor_render_ITextureBinding_H
