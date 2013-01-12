#ifndef traktor_render_IndexBufferDynamicDx11_H
#define traktor_render_IndexBufferDynamicDx11_H

#include "Render/Dx11/IndexBufferDx11.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;

/*!
 * \ingroup DX11
 */
class IndexBufferDynamicDx11 : public IndexBufferDx11
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferDynamicDx11 > create(ContextDx11* context, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferDynamicDx11();

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

private:
	bool m_locked;

	IndexBufferDynamicDx11(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferDynamicDx11_H
