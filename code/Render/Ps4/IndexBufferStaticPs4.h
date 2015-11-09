#ifndef traktor_render_IndexBufferStaticPs4_H
#define traktor_render_IndexBufferStaticPs4_H

#include "Render/Ps4/IndexBufferPs4.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;

/*!
 * \ingroup GNM
 */
class IndexBufferStaticPs4 : public IndexBufferPs4
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferStaticPs4 > create(ContextPs4* context, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferStaticPs4();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;

	IndexBufferStaticPs4(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferStaticPs4_H
