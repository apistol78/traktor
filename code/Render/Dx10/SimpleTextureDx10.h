#ifndef traktor_render_SimpleTextureDx10_H
#define traktor_render_SimpleTextureDx10_H

#include "Core/Misc/ComRef.h"
#include "Render/ISimpleTexture.h"

namespace traktor
{
	namespace render
	{

class ContextDx10;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup DX10
 */
class SimpleTextureDx10 : public ISimpleTexture
{
	T_RTTI_CLASS;

public:
	SimpleTextureDx10(ContextDx10* context);

	virtual ~SimpleTextureDx10();

	bool create(ID3D10Device* d3dDevice, const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual ITexture* resolve();

	virtual int getWidth() const;
	
	virtual int getHeight() const;

	virtual bool lock(int level, Lock& lock);

	virtual void unlock(int level);

	ID3D10ShaderResourceView* getD3D10TextureResourceView() const;

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Texture2D > m_d3dTexture;
	ComRef< ID3D10ShaderResourceView > m_d3dTextureResourceView;
	int m_width;
	int m_height;
};
		
	}
}

#endif	// traktor_render_SimpleTextureDx10_H
