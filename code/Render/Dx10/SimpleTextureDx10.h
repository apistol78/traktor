#ifndef traktor_render_SimpleTextureDx10_H
#define traktor_render_SimpleTextureDx10_H

#include "Render/SimpleTexture.h"
#include "Core/Heap/Ref.h"
#include "Core/Misc/ComRef.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX10_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextDx10;
struct SimpleTextureCreateDesc;
		
/*!
 * \ingroup DX10
 */
class T_DLLCLASS SimpleTextureDx10 : public SimpleTexture
{
	T_RTTI_CLASS(SimpleTexture)

public:
	SimpleTextureDx10(ContextDx10* context);

	virtual ~SimpleTextureDx10();

	bool create(ID3D10Device* d3dDevice, const SimpleTextureCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

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
