#ifndef traktor_render_VolumeTextureDx11_H
#define traktor_render_VolumeTextureDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/IVolumeTexture.h"

namespace traktor
{
	namespace render
	{
		
class ContextDx11;
struct VolumeTextureCreateDesc;

/*!
 * \ingroup DX11
 */
class VolumeTextureDx11 : public IVolumeTexture
{
	T_RTTI_CLASS;
	
public:
	VolumeTextureDx11(ContextDx11* context);

	virtual ~VolumeTextureDx11();

	bool create(const VolumeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture3D > m_d3dTexture;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int32_t m_width;
	int32_t m_height;
	int32_t m_depth;
};
		
	}
}

#endif	// traktor_render_VolumeTextureDx11_H
