#pragma once

#include "Render/ICubeTexture.h"
#include "Render/Types.h"
#include "Render/Vulkan/Private/ApiHeader.h"

namespace traktor
{
	namespace render
	{

class ApiBuffer;
class Context;
class Image;

struct CubeTextureCreateDesc;

/*!
 * \ingroup Vulkan
 */
class CubeTextureVk : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	explicit CubeTextureVk(Context* context, const CubeTextureCreateDesc& desc, uint32_t& instances);

	virtual ~CubeTextureVk();

	bool create(const wchar_t* const tag);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getSide() const override final;

	virtual int32_t getMips() const override final;

	virtual bool lock(int32_t side, int32_t level, Lock& lock) override final;

	virtual void unlock(int32_t side, int32_t level) override final;

	Image& getImage() const { return *m_textureImage; }

private:
	Context* m_context = nullptr;
	uint32_t& m_instances;
	Ref< ApiBuffer > m_stagingBuffer;
	Ref< Image > m_textureImage;
	CubeTextureCreateDesc m_desc;
};

	}
}
