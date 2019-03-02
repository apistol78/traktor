#pragma once

#include "Core/Ref.h"
#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{

class ContextPs4;
struct VolumeTextureCreateDesc;

/*!
 * \ingroup GNM
 */
class VolumeTexturePs4 : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	VolumeTexturePs4(ContextPs4* context);

	virtual ~VolumeTexturePs4();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

private:
	Ref< ContextPs4 > m_context;
};

	}
}
