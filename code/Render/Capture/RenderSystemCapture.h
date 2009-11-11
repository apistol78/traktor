#ifndef traktor_render_RenderSystemCapture_H
#define traktor_render_RenderSystemCapture_H

#include "Core/Heap/Ref.h"
#include "Render/IRenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_CAPTURE_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief Performance capture render system.
 * \ingroup RenderCapture
 *
 * This render system is only a wrapper around
 * a "real" render system implementation.
 * It will record statistics and frame captures for
 * debugging purposes only.
 */
class T_DLLCLASS RenderSystemCapture : public IRenderSystem
{
	T_RTTI_CLASS(RenderSystemCapture)

public:
	RenderSystemCapture(IRenderSystem* renderSystem);

	virtual bool create();

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

	virtual Ref< ProgramResource > compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate);

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource);

private:
	Ref< IRenderSystem > m_renderSystem;
};

	}
}

#endif	// traktor_render_RenderSystemCapture_H
