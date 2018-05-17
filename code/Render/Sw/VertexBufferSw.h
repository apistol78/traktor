/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_VertexBufferSw_H
#define traktor_render_VertexBufferSw_H

#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/VertexBuffer.h"

namespace traktor
{
	namespace render
	{

class VertexElement;

/*!
 * \ingroup SW
 */
class VertexBufferSw : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	typedef float vertex_tuple_t [4];

	VertexBufferSw(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	const AlignedVector< VertexElement >& getVertexElements() const { return m_vertexElements; }

	const uint32_t getVertexCount() const { return m_vertexCount; }

	const vertex_tuple_t* getData() { return m_data.ptr(); }

private:
	AlignedVector< VertexElement > m_vertexElements;
	uint32_t m_vertexStride;
	uint32_t m_vertexCount;
	AutoArrayPtr< vertex_tuple_t > m_data;
	uint8_t* m_lock;
	uint32_t m_lockOffset;
	uint32_t m_lockCount;
};
	
	}
}

#endif	// traktor_render_VertexBufferSw_H
