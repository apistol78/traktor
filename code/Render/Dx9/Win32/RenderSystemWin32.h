/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderSystemWin32_H
#define traktor_render_RenderSystemWin32_H

#include <list>
#include <map>
#include "Core/RefArray.h"
#include "Core/Misc/ComRef.h"
#include "Core/Thread/Semaphore.h"
#include "Render/IRenderSystem.h"
#include "Render/Dx9/Platform.h"
#include "Render/Dx9/Win32/Window.h"

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

class ClearTarget;
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

	virtual bool create(const RenderSystemDesc& desc);

	virtual void destroy();

	virtual bool reset(const RenderSystemDesc& desc);

	virtual void getInformation(RenderSystemInformation& outInfo) const;

	virtual uint32_t getDisplayModeCount() const;
	
	virtual DisplayMode getDisplayMode(uint32_t index) const;
	
	virtual DisplayMode getCurrentDisplayMode() const;

	virtual float getDisplayAspectRatio() const;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc);

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc);

	virtual Ref< VertexBuffer > createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc);
	
	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag);

	virtual Ref< IProgramCompiler > createProgramCompiler() const;

	virtual Ref< ITimeQuery > createTimeQuery() const;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const;

	virtual void purge();

	// \name Render view management
	// \{

	void addRenderView(RenderViewWin32* renderView);

	void removeRenderView(RenderViewWin32* renderView);

	/*! \brief Begin rendering frame.
	 *
	 * Called from render view in order to exclusively
	 * acquire device for rendering. No resources
	 * are permitted to be created during rendering
	 * except from rendering thread.
	 *
	 * \return True if rendering is permitted.
	 */
	bool beginRender();

	/*! \brief Finish rendering frame.
	 */
	void endRender(bool deviceLost);

	/*! \brief Try to recover lost device.
	 *
	 * \param True if device is recovered.
	 */
	bool tryRecoverDevice();

	/*! \brief Reset device.
	 *
	 * Release all non-managed resources and
	 * try to reset the device.
	 *
	 * \return Reset error code.
	 */
	HRESULT resetDevice();

private:
	ComRef< IDirect3D9 > m_d3d;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	D3DPRESENT_PARAMETERS m_d3dPresent;
	D3DDISPLAYMODE m_d3dDefaultDisplayMode;
	Ref< Window > m_window;
	Ref< ResourceManagerDx9 > m_resourceManager;
	Ref< ShaderCache > m_shaderCache;
	Ref< ClearTarget > m_clearTarget;
	Ref< ParameterCache > m_parameterCache;
	Ref< VertexDeclCache > m_vertexDeclCache;
	int32_t m_maxAnisotropy;
	RefArray< RenderViewWin32 > m_renderViews;
	Semaphore m_renderLock;
	bool m_inRender;
	bool m_deviceLost;
};

	}
}

#endif	// traktor_render_RenderSystemWin32_H
