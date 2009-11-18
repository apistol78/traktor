#ifndef traktor_render_SimpleTextureDx9_H
#define traktor_render_SimpleTextureDx9_H

#include "Render/ISimpleTexture.h"
#include "Render/Dx9/TextureBaseDx9.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx9;
struct SimpleTextureCreateDesc;

/*!
 * \ingroup DX9 Xbox360
 */
class T_DLLCLASS SimpleTextureDx9
:	public ISimpleTexture
,	public TextureBaseDx9
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx9(ContextDx9* context);

	virtual ~SimpleTextureDx9();

	bool create(IDirect3DDevice9* d3dDevice, const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	virtual IDirect3DBaseTexture9* getD3DBaseTexture() const;

private:
	Ref< ContextDx9 > m_context;
	ComRef< IDirect3DTexture9 > m_d3dTexture;
	TextureFormat m_format;
	int m_width;
	int m_height;
	void* m_lock;
};
		
	}
}

#endif	// traktor_render_SimpleTextureDx9_H
