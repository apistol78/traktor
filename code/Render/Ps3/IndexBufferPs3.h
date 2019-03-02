#pragma once

#include "Render/IndexBuffer.h"

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

class T_DLLCLASS IndexBufferPs3 : public IndexBuffer
{
	T_RTTI_CLASS;

public:
	IndexBufferPs3(MemoryHeapObject* ibo, IndexType indexType, int bufferSize, int32_t& counter);

	virtual ~IndexBufferPs3();

	virtual void destroy() override final;

	virtual void* lock() override final;

	virtual void unlock() override final;

	uint8_t getLocation() const;

	uint32_t getOffset() const;

private:
	MemoryHeapObject* m_ibo;
	int32_t& m_counter;
};

	}
}

