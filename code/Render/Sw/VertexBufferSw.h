#ifndef traktor_render_VertexBufferSw_H
#define traktor_render_VertexBufferSw_H

#include <vector>
#include "Core/Math/Vector4.h"
#include "Core/Misc/AutoPtr.h"
#include "Render/VertexBuffer.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class VertexElement;

/*!
 * \ingroup SW
 */
class T_DLLCLASS VertexBufferSw : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	typedef float vertex_tuple_t [4];

	VertexBufferSw(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	const std::vector< VertexElement >& getVertexElements() const { return m_vertexElements; }

	const uint32_t getVertexCount() const { return m_vertexCount; }

	const vertex_tuple_t* getData() { return m_data.ptr(); }

private:
	std::vector< VertexElement > m_vertexElements;
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
