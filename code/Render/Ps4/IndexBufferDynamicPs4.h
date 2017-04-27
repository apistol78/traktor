/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferDynamicPs4_H
#define traktor_render_IndexBufferDynamicPs4_H

#include "Render/Ps4/IndexBufferPs4.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;

/*!
 * \ingroup GNM
 */
class IndexBufferDynamicPs4 : public IndexBufferPs4
{
	T_RTTI_CLASS;

public:
	static Ref< IndexBufferDynamicPs4 > create(ContextPs4* context, IndexType indexType, uint32_t bufferSize);

	virtual ~IndexBufferDynamicPs4();

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void* lock() T_OVERRIDE T_FINAL;
	
	virtual void unlock() T_OVERRIDE T_FINAL;

	virtual void prepare() T_OVERRIDE T_FINAL;

private:
	Ref< ContextPs4 > m_context;

	IndexBufferDynamicPs4(IndexType indexType, uint32_t bufferSize);
};
	
	}
}

#endif	// traktor_render_IndexBufferDynamicPs4_H
