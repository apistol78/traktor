#pragma once

#include "Render/IVolumeTexture.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class CommandBufferPool;
class Context;
class Queue;
struct VolumeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class VolumeTextureVk : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	explicit VolumeTextureVk(Context* context);

	virtual ~VolumeTextureVk();

	bool create(const VolumeTextureCreateDesc& desc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	VkImage getVkImage() const { return m_textureImage; }

	VkImageView getVkImageView() const { return m_textureView; }

private:
	Ref< Context > m_context;
	VkImage m_textureImage;
	VkImageView m_textureView;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
};

	}
}
