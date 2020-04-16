#pragma once

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{

/*!
 * \ingroup Vrfy
 */
class VolumeTextureVrfy : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	VolumeTextureVrfy(IVolumeTexture* texture);

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getMips() const override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	IVolumeTexture* getTexture() const { return m_texture; }

private:
	Ref< IVolumeTexture > m_texture;
};

	}
}
