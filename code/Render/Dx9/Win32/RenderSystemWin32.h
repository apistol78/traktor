#ifndef traktor_render_RenderSystemWin32_H
#define traktor_render_RenderSystemWin32_H

#include <list>
#include <map>
#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Core/Thread/Semaphore.h"
#include "Render/IRenderSystem.h"
#include "Render/Dx9/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_DX9_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ParameterCache;
class ProgramWin32;
class RenderViewWin32;
class ResourceManagerDx9;
class ShaderCache;
class VertexDeclCache;

/*! \brief DirectX 9 render system.
 * \ingroup DX9
 *
 * DX9 render system implementation.
 */
class T_DLLCLASS RenderSystemWin32
:	public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemWin32();

	virtual bool create(const RenderSystemCreateDesc& desc);

	virtual void destroy();

	virtual int getDisplayModeCount() const;
	
	virtual Ref< DisplayMode > getDisplayMode(int index);
	
	virtual Ref< DisplayMode > getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual Ref< IRenderView > createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual Ref< IRenderView > createRenderView(void* windowHandle, const RenderViewCreateDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

	// \name Render view management
	// \{

	void addRenderView(RenderViewWin32* renderView);

	void removeRenderView(RenderViewWin32* renderView);

	bool beginRender();

	void endRender();

	// \}

private:
	ComRef< IDirect3D9 > m_d3d;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	D3DPRESENT_PARAMETERS m_d3dPresentNull;
	D3DDISPLAYMODE m_d3dDefaultDisplayMode;
	HWND m_hWnd;
	Ref< ResourceManagerDx9 > m_resourceManager;
	Ref< ShaderCache > m_shaderCache;
	Ref< ParameterCache > m_parameterCache;
	Ref< VertexDeclCache > m_vertexDeclCache;
	RefArray< RenderViewWin32 > m_renderViews;
	Semaphore m_renderLock;
	float m_mipBias;

	HRESULT resetDevice();

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_RenderSystemWin32_H
