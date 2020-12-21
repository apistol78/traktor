#pragma once

#include "Render/ISimpleTexture.h"
#include "Render/Vulkan/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class CommandBufferPool;
class Context;
class Queue;
struct SimpleTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class SimpleTextureVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	explicit SimpleTextureVk(
		Context* context,
		Queue* graphicsQueue,
		CommandBufferPool* graphicsCommandPool
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
	Ref< Context > m_context;
	Ref< Queue > m_graphicsQueue;
	Ref< CommandBufferPool > m_graphicsCommandPool;
	VmaAllocation m_stagingBufferAllocation;
	VkBuffer m_stagingBuffer;
	VmaAllocation m_textureAllocation;
	VkImage m_textureImage;
	VkImageView m_textureView;
	SimpleTextureCreateDesc m_desc;
};

	}
}
