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

struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetVk(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator
	);

	virtual ~RenderTargetVk();

	bool createPrimary(int32_t width, int32_t height, VkFormat format, VkImage image, const wchar_t* const tag);

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	void prepareAsTarget(VkCommandBuffer cmdBuffer);

	void prepareForPresentation(VkCommandBuffer cmdBuffer);

	void prepareAsTexture(VkCommandBuffer cmdBuffer);

	void prepareForReadBack(VkCommandBuffer cmdBuffer);

	void discard();

	VkFormat getVkFormat() const { return m_format; }

	VkImage getVkImage() const { return m_image; }

	VkImageView getVkImageView() const { return m_imageView; }

	VkImageLayout getVkImageLayout() const { return m_imageLayout; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	VkFormat m_format;
	VkImage m_image;
	VmaAllocation m_allocation;
	VkImageView m_imageView;
	VkImageLayout m_imageLayout;
	VkAccessFlags m_accessFlags;
	int32_t m_width;
	int32_t m_height;
};

	}
}
