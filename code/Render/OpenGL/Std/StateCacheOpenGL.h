#ifndef traktor_render_StateCacheOpenGL_H
#define traktor_render_StateCacheOpenGL_H

#include "Core/Object.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/TypesOpenGL.h"

namespace traktor
{
	namespace render
	{

class StateCacheOpenGL : public Object
{
	T_RTTI_CLASS;
	
public:
	StateCacheOpenGL();

	void forceRenderState(const RenderState& renderState, bool invertCull);

	void setRenderState(const RenderState& renderState, bool invertCull);
	
	void setColorMask(uint32_t colorMask);
	
	void setDepthMask(GLboolean depthMask);

	void setPermitDepth(bool permitDepth);

	void validate();
	
private:
	RenderState m_shadowRenderState;
	bool m_permitDepth;
};
	
	}
}

#endif	// traktor_render_StateCacheOpenGL_H
