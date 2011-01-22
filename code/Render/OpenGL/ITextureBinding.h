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
	virtual void bind(GLuint unit, const SamplerState& samplerState, GLint locationTexture, GLint locationOffset) = 0;
};
	
	}
}

#endif	// traktor_render_ITextureBinding_H
