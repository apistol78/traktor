#ifndef traktor_render_RenderSystemOpenGLES2_H
#define traktor_render_RenderSystemOpenGLES2_H

#include "Render/IRenderSystem.h"
#include "Render/OpenGL/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class ContextOpenGLES2;

/*! \brief OpenGL ES 2.0 render system.
 * \ingroup OGL
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemOpenGLES2 : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemOpenGLES2();

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

	virtual void purge();

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const;

private:
	void* m_nativeHandle;
	Ref< ContextOpenGLES2 > m_context;
};

	}
}

#endif	// traktor_render_RenderSystemOpenGLES2_H
