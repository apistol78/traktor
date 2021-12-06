#pragma once

#include "Render/ISimpleTexture.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class ApiBuffer;
class Context;
class Image;

struct SimpleTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class SimpleTextureVk : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	explicit SimpleTextureVk(Context* context, uint32_t& instances);

	virtual ~SimpleTextureVk();

	bool create(
		const SimpleTextureCreateDesc& desc,
		const wchar_t* const tag
	);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t level) override final;

	virtual void* getInternalHandle() override final;

	Image& getImage() const { return *m_textureImage; }

private:
	Context* m_context = nullptr;
	uint32_t& m_instances;
	Ref< ApiBuffer > m_stagingBuffer;
	Ref< Image > m_textureImage;
	SimpleTextureCreateDesc m_desc;
};

	}
}
