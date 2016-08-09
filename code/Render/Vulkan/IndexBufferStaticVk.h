#ifndef traktor_render_IndexBufferStaticVk_H
#define traktor_render_IndexBufferStaticVk_H

#include "Render/Vulkan/IndexBufferVk.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class IndexBufferStaticVk : public IndexBufferVk
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferStaticVk > create(IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferStaticVk();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	IndexBufferStaticVk(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferStaticVk_H
