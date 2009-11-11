#ifndef traktor_render_RenderSystemOpenGLES2_H
#define traktor_render_RenderSystemOpenGLES2_H

#include "Core/Heap/Ref.h"
#include "Render/IRenderSystem.h"
#include "Render/OpenGL/Platform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_ES2_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
	T_RTTI_CLASS(RenderSystemOpenGLES2)

public:
	RenderSystemOpenGLES2();

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
	Ref< ContextOpenGLES2 > m_globalContext;
#if defined(T_OPENGL_ES2_HAVE_EGL)
	EGLDisplay m_display;
	EGLContext m_context;
	EGLSurface m_surface;
#endif
#if defined(_WIN32)
	HWND m_hWnd;

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
};

	}
}

#endif	// traktor_render_RenderSystemOpenGLES2_H
