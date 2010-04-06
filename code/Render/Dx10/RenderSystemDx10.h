#ifndef traktor_render_RenderSystemDx10_H
#define traktor_render_RenderSystemDx10_H

#include "Core/RefArray.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Misc/ComRef.h"
#include "Render/IRenderSystem.h"
#include "Render/Dx10/Platform.h"

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

/*! \brief DirectX 10 render system.
 * \ingroup DX10
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemDx10 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemDx10();

	virtual bool create(const RenderSystemCreateDesc& desc);

	virtual void destroy();

	virtual uint32_t getDisplayModeCount() const;
	
	virtual DisplayMode getDisplayMode(uint32_t index) const;
	
	virtual DisplayMode getCurrentDisplayMode() const;

	virtual bool handleMessages();

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc);

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

private:
	Ref< ContextDx10 > m_context;
	ComRef< ID3D10Device > m_d3dDevice;
	ComRef< IDXGIFactory > m_dxgiFactory;
	AutoArrayPtr< DXGI_MODE_DESC > m_dxgiDisplayModes;
	std::vector< DisplayMode > m_displayModes;
	HWND m_hWnd;
	float m_mipBias;

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_RenderSystemDx10_H
