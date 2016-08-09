#ifndef traktor_render_IndexBufferDynamicVk_H
#define traktor_render_IndexBufferDynamicVk_H

#include "Render/Vulkan/IndexBufferVk.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class IndexBufferDynamicVk : public IndexBufferVk
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferDynamicVk > create(IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferDynamicVk();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	IndexBufferDynamicVk(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferDynamicVk_H
