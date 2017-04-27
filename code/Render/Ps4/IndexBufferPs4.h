/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferPs4_H
#define traktor_render_IndexBufferPs4_H

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup GNM
 */
class IndexBufferPs4 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferPs4(IndexType indexType, uint32_t bufferSize);

	virtual void prepare();
};
	
	}
}

#endif	// traktor_render_IndexBufferPs4_H
