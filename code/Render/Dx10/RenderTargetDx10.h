#ifndef traktor_render_RenderTargetDx10_H
#define traktor_render_RenderTargetDx10_H

#include "Render/Texture.h"
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
struct RenderTargetSetCreateDesc;
struct RenderTargetCreateDesc;

/*!
 * \ingroup DX10
 */
class T_DLLCLASS RenderTargetDx10 : public Texture
{
	T_RTTI_CLASS(RenderTargetDx10)

public:
	RenderTargetDx10(ContextDx10* context);

	virtual ~RenderTargetDx10();

	bool create(ID3D10Device* d3dDevice, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	inline ID3D10RenderTargetView* getD3D10RenderTargetView() const {
		return m_d3dRenderTargetView;
	}

	inline ID3D10ShaderResourceView* getD3D10TextureResourceView() const {
		return m_d3dTextureResourceView;
	}

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Texture2D > m_d3dTexture;
	ComRef< ID3D10RenderTargetView > m_d3dRenderTargetView;
	ComRef< ID3D10ShaderResourceView > m_d3dTextureResourceView;
	int m_width;
	int m_height;
};

	}
}

#endif	// traktor_render_RenderTargetDx10_H
