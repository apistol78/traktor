#ifndef traktor_render_VertexBufferVk_H
#define traktor_render_VertexBufferVk_H

#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class VertexBufferVk : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	virtual void prepare();

protected:
	VertexBufferVk(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferVk_H
