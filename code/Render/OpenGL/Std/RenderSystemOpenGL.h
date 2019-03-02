#pragma once

#include "Render/IRenderSystem.h"
#include "Render/OpenGL/Std/Platform.h"
#include "Render/OpenGL/Std/ResourceContextOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

#if defined(__LINUX__)
struct _XRRScreenResources;
#endif

namespace traktor
{
	namespace render
	{

class Window;

/*! \brief OpenGL render system.
 * \ingroup OGL
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemOpenGL : public IRenderSystem
{
	T_RTTI_CLASS;

public:
	RenderSystemOpenGL();

	virtual bool create(const RenderSystemDesc& desc) override final;

	virtual void destroy() override final;

	virtual bool reset(const RenderSystemDesc& desc) override final;

	virtual void getInformation(RenderSystemInformation& outInfo) const override final;

	virtual uint32_t getDisplayModeCount() const override final;

	virtual DisplayMode getDisplayMode(uint32_t index) const override final;

	virtual DisplayMode getCurrentDisplayMode() const override final;

	virtual float getDisplayAspectRatio() const override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewDefaultDesc& desc) override final;

	virtual Ref< IRenderView > createRenderView(const RenderViewEmbeddedDesc& desc) override final;

	virtual Ref< VertexBuffer > createVertexBuffer(const AlignedVector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< IndexBuffer > createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic) override final;

	virtual Ref< ISimpleTexture > createSimpleTexture(const SimpleTextureCreateDesc& desc) override final;

	virtual Ref< ICubeTexture > createCubeTexture(const CubeTextureCreateDesc& desc) override final;

	virtual Ref< IVolumeTexture > createVolumeTexture(const VolumeTextureCreateDesc& desc) override final;

	virtual Ref< RenderTargetSet > createRenderTargetSet(const RenderTargetSetCreateDesc& desc) override final;

	virtual Ref< IProgram > createProgram(const ProgramResource* programResource, const wchar_t* const tag) override final;

	virtual Ref< ITimeQuery > createTimeQuery() const override final;

	virtual void purge() override final;

	virtual void getStatistics(RenderSystemStatistics& outStatistics) const override final;

private:
	friend class RenderViewOpenGL;

	RenderSystemInformation m_info;

#if defined(_WIN32)

	Ref< Window > m_windowShared;
	Ref< Window > m_window;

#elif defined(__APPLE__)

	void* m_windowHandle;

#else   // LINUX

	::Display* m_display;
	Ref< Window > m_windowShared;
    Ref< Window > m_window;
	struct _XRRScreenResources* m_screenResources;

#endif

	Ref< ResourceContextOpenGL > m_resourceContext;
	bool m_useProgramCache;
};

	}
}

