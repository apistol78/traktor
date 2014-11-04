#ifndef traktor_render_RenderSystemXbox360_H
#define traktor_render_RenderSystemXbox360_H

#include <list>
#include "Core/Misc/ComRef.h"
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
class VertexDeclCache;
class RenderTargetPool;
class RenderViewXbox360;
class ResourceManagerDx9;

/*! \brief DirectX 9 (XBOX360) render system.
 * \ingroup Xbox360
 *
 * DX9 render system implementation.
 */
class T_DLLCLASS RenderSystemXbox360 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemXbox360();

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

	/*! \name Direct3D interface. */
	//@{

	IDirect3D9* getD3D() const { return m_d3d; }

	//@}

private:
	ComRef< IDirect3D9 > m_d3d;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	Ref< ResourceManagerDx9 > m_resourceManager;
	ParameterCache* m_parameterCache;
	VertexDeclCache* m_vertexDeclCache;
	Ref< RenderTargetPool > m_renderTargetPool;
	Ref< RenderViewXbox360 > m_renderView;
	std::vector< ComRef< IDirect3DResource9 > > m_deferredRelease;
};

	}
}

#endif	// traktor_render_RenderSystemXbox360_H
