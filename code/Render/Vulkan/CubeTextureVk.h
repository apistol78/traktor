#pragma once

#include "Render/ICubeTexture.h"
#include "Render/Types.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class CommandBufferPool;
class Queue;

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
		Queue* graphicsQueue,
		CommandBufferPool* graphicsCommandPool,
		const CubeTextureCreateDesc& desc
	);

	virtual ~CubeTextureVk();

	bool create(const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getSide() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	VkImage getVkImage() const { return m_textureImage; }

	VkImageView getVkImageView() const { return m_textureView; }

private:
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VmaAllocator m_allocator;
	Ref< Queue > m_graphicsQueue;
	Ref< CommandBufferPool > m_graphicsCommandPool;
	CubeTextureCreateDesc m_desc;
	VmaAllocation m_textureImageAllocation;
	VkImage m_textureImage;
	VkImageView m_textureView;
	VmaAllocation m_stagingBufferAllocation;
	VkBuffer m_stagingBuffer;
};

	}
}
