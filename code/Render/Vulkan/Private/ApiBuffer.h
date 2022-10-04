#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor::render
{

class Context;

/*!
 * \ingroup Vulkan
 */
class ApiBuffer : public Object
{
	T_RTTI_CLASS;

public:
	ApiBuffer() = delete;

	ApiBuffer(const ApiBuffer&) = delete;

	ApiBuffer(ApiBuffer&&) = delete;

	explicit ApiBuffer(Context* context);

	virtual ~ApiBuffer();

	bool create(uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess);

	void destroy();

	void* lock();

	void unlock();

	operator VkBuffer () const { return m_buffer; }

private:
	Context* m_context = nullptr;
	VmaAllocation m_allocation = 0;
	VkBuffer m_buffer = 0;
	void* m_locked = nullptr;
};
		
}
