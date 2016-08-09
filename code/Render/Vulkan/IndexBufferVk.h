#ifndef traktor_render_IndexBufferVk_H
#define traktor_render_IndexBufferVk_H

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class IndexBufferVk : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferVk(IndexType indexType, uint32_t bufferSize);

	virtual void prepare();
};
	
	}
}

#endif	// traktor_render_IndexBufferVk_H
