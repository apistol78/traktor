#pragma once

#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{

class ResourceTracker;

/*!
 * \ingroup Vrfy
 */
class VolumeTextureVrfy : public IVolumeTexture
{
	T_RTTI_CLASS;

public:
	explicit VolumeTextureVrfy(ResourceTracker* resourceTracker, IVolumeTexture* texture);

	virtual ~VolumeTextureVrfy();

	virtual void destroy() override final;

	virtual ITexture* resolve() override final;

	virtual int32_t getWidth() const override final;

	virtual int32_t getHeight() const override final;

	virtual int32_t getDepth() const override final;

	virtual int32_t getMips() const override final;

	IVolumeTexture* getTexture() const { return m_texture; }

private:
	Ref< ResourceTracker > m_resourceTracker;
	Ref< IVolumeTexture > m_texture;
};

	}
}
