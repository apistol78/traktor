#ifndef traktor_render_SimpleTextureDx11_H
#define traktor_render_SimpleTextureDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup DX11
 */
class SimpleTextureDx11 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx11(ContextDx11* context);

	virtual ~SimpleTextureDx11();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual ITexture* resolve() T_OVERRIDE T_FINAL;

	virtual int getWidth() const T_OVERRIDE T_FINAL;
	
	virtual int getHeight() const T_OVERRIDE T_FINAL;
	
	virtual bool lock(int level, Lock& lock) T_OVERRIDE T_FINAL;

	virtual void unlock(int level) T_OVERRIDE T_FINAL;

	virtual void* getInternalHandle() T_OVERRIDE T_FINAL;

	ID3D11ShaderResourceView* getD3D11TextureResourceView() const;

private:
	Ref< ContextDx11 > m_context;
	ComRef< ID3D11Texture2D > m_d3dTexture;
	ComRef< ID3D11ShaderResourceView > m_d3dTextureResourceView;
	int m_width;
	int m_height;
};
		
	}
}

#endif	// traktor_render_SimpleTextureDx11_H
