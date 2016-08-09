#ifndef traktor_render_VertexBufferDynamicVk_H
#define traktor_render_VertexBufferDynamicVk_H

#include "Render/VertexElement.h"
#include "Render/Vulkan/VertexBufferVk.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vulkan
 */
class VertexBufferDynamicVk : public VertexBufferVk
{
	T_RTTI_CLASS;

public:
	static Ref< VertexBufferDynamicVk > create(
		uint32_t bufferSize,
		const std::vector< VertexElement >& vertexElements
	);

	virtual ~VertexBufferDynamicVk();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount) T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	VertexBufferDynamicVk(uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_VertexBufferDynamicVk_H
