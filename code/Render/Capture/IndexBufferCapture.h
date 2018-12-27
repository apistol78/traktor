/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferCapture_H
#define traktor_render_IndexBufferCapture_H

#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Capture
 */
class IndexBufferCapture : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferCapture(IndexBuffer* indexBuffer, IndexType indexType, uint32_t bufferSize);

	virtual void destroy() override final;

	virtual void* lock() override final;
	
	virtual void unlock() override final;

	IndexBuffer* getIndexBuffer() const { return m_indexBuffer; }

private:
	Ref< IndexBuffer > m_indexBuffer;
	bool m_locked;
};
	
	}
}

#endif	// traktor_render_IndexBufferCapture_H
