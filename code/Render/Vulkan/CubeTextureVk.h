#pragma once

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__LINUX__)
#	define VK_USE_PLATFORM_LINUX_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR
#	define VK_NO_PROTOTYPES
#	include <vulkan.h>
#endif

#include "Render/ICubeTexture.h"

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
	CubeTextureVk();

	virtual ~CubeTextureVk();

	bool create(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkCommandPool commandPool,
		VkQueue queue,
		const CubeTextureCreateDesc& desc
	);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	VkImage getVkImage() const { return m_textureImage; }

	VkImageView getVkImageView() const { return m_textureView; }

private:
	VkImage m_textureImage;
	VkImageView m_textureView;
	int32_t m_mips;
	int32_t m_side;
};

	}
}
