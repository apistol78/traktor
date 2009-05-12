#ifndef traktor_render_VertexBufferSw_H
#define traktor_render_VertexBufferSw_H

#include <vector>
#include "Core/Heap/Ref.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"
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
	T_RTTI_CLASS(VertexBufferSw)

public:
	VertexBufferSw(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize);

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);
	
	virtual void unlock();

	const std::vector< VertexElement >& getVertexElements() const { return m_vertexElements; }

	const Vector4* getData() const { return &m_data[0]; }

private:
	std::vector< VertexElement > m_vertexElements;
	uint32_t m_vertexStride;
	AlignedVector< Vector4 > m_data;
	uint8_t* m_lock;
	uint32_t m_lockOffset;
	uint32_t m_lockCount;
};
	
	}
}

#endif	// traktor_render_VertexBufferSw_H
