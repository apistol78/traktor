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
#endif
#include <vk_mem_alloc.h>

#include "Render/ICubeTexture.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

struct CubeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class CubeTextureVk : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureVk(
		VkPhysicalDevice physicalDevice,
		VkDevice logicalDevice,
		VmaAllocator allocator,
		VkCommandPool setupCommandPool,
		VkQueue setupQueue,
		const CubeTextureCreateDesc& desc
	);

	virtual ~CubeTextureVk();

	bool create();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	virtual bool copy(int32_t side, int32_t level, const ISimpleTexture* sourceTexture) override final;

	VkImage getVkImage() const { return m_textureImage; }

	VkImageView getVkImageView() const { return m_textureView; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	VkCommandPool m_setupCommandPool;
	VkQueue m_setupQueue;
	CubeTextureCreateDesc m_desc;
	VmaAllocation m_textureImageAllocation;
	VkImage m_textureImage;
	VkImageView m_textureView;
	VmaAllocation m_stagingBufferAllocation;
	VkBuffer m_stagingBuffer;
};

	}
}
