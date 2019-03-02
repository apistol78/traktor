#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class MemoryHeapObject;
class StateCachePs3;

class T_DLLCLASS VertexBufferPs3 : public VertexBuffer
{
	T_RTTI_CLASS;

public:
	VertexBufferPs3(const AlignedVector< VertexElement >& vertexElements, MemoryHeapObject* vbo, int bufferSize, int32_t& counter);

	virtual ~VertexBufferPs3();

	virtual void destroy();

	virtual void* lock();

	virtual void* lock(uint32_t vertexOffset, uint32_t vertexCount);

	virtual void unlock();

	void bind(StateCachePs3& stateCache, const AlignedVector< uint8_t >& signature);

private:
	struct AttributeDesc
	{
		uint8_t size;
		uint8_t type;
		uint32_t offset;
	};

	static VertexBufferPs3* ms_activeVertexBuffer;
	MemoryHeapObject* m_vbo;
	uint8_t m_vertexStride;
	AttributeDesc m_attributeDesc[16];
	int32_t& m_counter;
};

	}
}

