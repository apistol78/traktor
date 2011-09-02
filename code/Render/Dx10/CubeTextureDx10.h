#ifndef traktor_render_CubeTextureDx10_H
#define traktor_render_CubeTextureDx10_H

#include "Core/Misc/ComRef.h"
#include "Render/ICubeTexture.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx10;
struct CubeTextureCreateDesc;

/*!
 * \ingroup DX10
 */
class T_DLLCLASS CubeTextureDx10 : public ICubeTexture
{
	T_RTTI_CLASS;

public:
	CubeTextureDx10(ContextDx10* context);

	virtual ~CubeTextureDx10();

	bool create(ID3D10Device* d3dDevice, const CubeTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int side, int level, Lock& lock);

	virtual void unlock(int side, int level);

	ID3D10ShaderResourceView* getD3D10TextureResourceView() const;

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Texture2D > m_d3dTexture;
	ComRef< ID3D10ShaderResourceView > m_d3dTextureResourceView;
	int m_side;
	int m_mipCount;
};
		
	}
}

#endif	// traktor_render_CubeTextureDx10_H
