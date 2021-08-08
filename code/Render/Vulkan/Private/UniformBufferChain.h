#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Core/Memory/BlockAllocator.h"

namespace traktor
{
	namespace render
	{

class Buffer;
class Context;
class UniformBufferChain;

struct UniformBufferRange
{
	UniformBufferChain* chain = nullptr;
	uint32_t offset = 0;
	void* ptr = nullptr;
};

class UniformBufferChain : public Object
{
public:
	static Ref< UniformBufferChain > create(Context* context, uint32_t blockCount, uint32_t blockSize);

	void destroy();

	bool allocate(UniformBufferRange& outRange);

	void free(const UniformBufferRange& range);

	Buffer* getBuffer() const { return m_buffer; }

private:
	Ref< Buffer > m_buffer;
	BlockAllocator m_allocator;

	explicit UniformBufferChain(Buffer* buffer, void* top, uint32_t blockCount, uint32_t blockSize);
};

	}
}
