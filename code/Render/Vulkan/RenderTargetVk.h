#pragma once

#include "Render/ISimpleTexture.h"
#include "Render/Vulkan/Private/ApiHeader.h"
#include "Render/Vulkan/Private/Image.h"

namespace traktor::render
{

class Context;
class CommandBuffer;
struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup Vulkan
 */
class RenderTargetVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	RenderTargetVk() = delete;

	explicit RenderTargetVk(Context* context);

	virtual ~RenderTargetVk();

	/*! Create primary target.
	 *
	 * \param width Width of target.
	 * \param height Height of target.
	 * \param format Pixel format of target.
	 * \param multiSample Number of samples.
	 * \param swapChainImage Swap chain target image.
	 * \param tag Debug tag.
	 * \return True if target successfully created.
	 */
	bool createPrimary(
		int32_t width,
		int32_t height,
		uint32_t multiSample,
		VkFormat format,
		VkImage swapChainImage,
		const wchar_t* const tag
	);

	bool create(const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc, const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	void prepareForPresentation(CommandBuffer* commandBuffer);

	VkFormat getVkFormat() const { return m_format; }

	Image* getImageTarget() const { return m_imageTarget; }

	Image* getImageResolved() const { return m_imageResolved; }

private:
	friend class RenderTargetSetVk;

	Context* m_context = nullptr;
	VkFormat m_format = VK_FORMAT_UNDEFINED;
	Ref< Image > m_imageTarget;
	Ref< Image > m_imageResolved;	//!< For MSAA targets; target image is resolved to this image after rendering.
	int32_t m_width = 0;
	int32_t m_height = 0;

	void prepareAsTarget(CommandBuffer* commandBuffer);

	void prepareAsTexture(CommandBuffer* commandBuffer);

	void prepareForReadBack(CommandBuffer* commandBuffer);
};

}
