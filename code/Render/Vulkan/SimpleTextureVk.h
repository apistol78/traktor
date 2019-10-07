#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#elif defined(__APPLE__)
#	define VK_USE_PLATFORM_MACOS_MVK
#	define VK_NO_PROTOTYPES
#	include <vulkan/vulkan.h>
#endif
#include <vk_mem_alloc.h>

#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

struct SimpleTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class SimpleTextureVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureVk(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator
	);

	virtual ~SimpleTextureVk();

	bool create(
		VkCommandPool commandPool,
		VkQueue queue,
		const SimpleTextureCreateDesc& desc,
		const wchar_t* const tag
	);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	VkImage getVkImage() const { return m_textureImage; }

	VkImageView getVkImageView() const { return m_textureView; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	VmaAllocation m_allocation;
	VkImage m_textureImage;
	VkImageView m_textureView;
	int32_t m_mips;
	int32_t m_width;
	int32_t m_height;
};

	}
}
