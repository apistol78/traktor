#ifndef traktor_render_SimpleTextureDx11_H
#define traktor_render_SimpleTextureDx11_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX11_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx11;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup DX11
 */
class T_DLLCLASS SimpleTextureDx11 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx11(ContextDx11* context);

	virtual ~SimpleTextureDx11();

	bool create(const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

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
