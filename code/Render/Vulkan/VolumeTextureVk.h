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

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{

struct VolumeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class VolumeTextureVk : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	VolumeTextureVk();

	virtual ~VolumeTextureVk();

	bool create(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue queue,
		const VolumeTextureCreateDesc& desc,
		const wchar_t* const tag
	);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	VkImage getVkImage() const { return m_textureImage; }

	VkImageView getVkImageView() const { return m_textureView; }

private:
	VkImage m_textureImage;
	VkImageView m_textureView;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
};

	}
}
