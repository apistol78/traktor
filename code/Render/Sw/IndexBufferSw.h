/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_IndexBufferSw_H
#define traktor_render_IndexBufferSw_H

#include <vector>
#include "Render/IndexBuffer.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup SW
 */
class IndexBufferSw : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferSw(IndexType indexType, uint32_t bufferSize);

	virtual void destroy();

	virtual void* lock();
	
	virtual void unlock();

	const std::vector< uint32_t >& getIndices() const;
	
private:
	std::vector< uint32_t > m_indices;
	void* m_lock;
};
	
	}
}

#endif	// traktor_render_IndexBufferSw_H
