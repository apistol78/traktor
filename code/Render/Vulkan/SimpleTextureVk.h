#pragma once

#include "Render/ISimpleTexture.h"
#include "Render/Vulkan/ApiHeader.h"

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
		VmaAllocator allocator,
		VkCommandPool setupCommandPool,
		VkQueue setupQueue
	);

	virtual ~SimpleTextureVk();

	bool create(
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
	VkCommandPool m_setupCommandPool;
	VkQueue m_setupQueue;
	VmaAllocation m_stagingBufferAllocation;
	VkBuffer m_stagingBuffer;
	VmaAllocation m_textureAllocation;
	VkImage m_textureImage;
	VkImageView m_textureView;
	SimpleTextureCreateDesc m_desc;
};

	}
}
