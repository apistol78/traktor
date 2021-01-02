#include "Core/Config.h"
#include "Render/Vulkan/ApiLoader.h"
#include "Render/Vulkan/Buffer.h"
#include "Render/Vulkan/Context.h"

namespace traktor
{
    namespace render
    {

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Buffer", Buffer, Object)

Buffer::Buffer(Context* context)
:	m_context(context)
{
}

Buffer::~Buffer()
{
	T_FATAL_ASSERT_M(m_context == nullptr, L"Not properly destroyed.");
}

bool Buffer::create(uint32_t bufferSize, uint32_t usageBits, bool cpuAccess, bool gpuAccess)
{
	T_FATAL_ASSERT(m_buffer == 0);

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.size = bufferSize;
	bci.usage = usageBits;
	bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo aci = {};
    if (cpuAccess && gpuAccess)
	    aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
    else if (!cpuAccess && gpuAccess)
        aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    else if (cpuAccess && !gpuAccess)
        aci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    else
        return false;

	if (vmaCreateBuffer(m_context->getAllocator(), &bci, &aci, &m_buffer, &m_allocation, nullptr) != VK_SUCCESS)
		return false;

    return true;
}

void Buffer::destroy()
{
	T_FATAL_ASSERT(m_locked == nullptr);
	if (m_buffer != 0)
	{
		m_context->addDeferredCleanup([
			buffer = m_buffer,
			allocation = m_allocation
		](Context* cx) {
			vmaDestroyBuffer(cx->getAllocator(), buffer, allocation);
		});

		m_allocation = 0;
		m_buffer = 0;
		m_context = nullptr;
	}
}

void* Buffer::lock()
{
	if (m_locked)
		return m_locked;

	if (vmaMapMemory(m_context->getAllocator(), m_allocation, &m_locked) != VK_SUCCESS)
		m_locked = nullptr;

	return m_locked;
}

void Buffer::unlock()
{
	if (m_locked)
	{
		vmaUnmapMemory(m_context->getAllocator(), m_allocation);
		m_locked = nullptr;
	}
}

    }
}