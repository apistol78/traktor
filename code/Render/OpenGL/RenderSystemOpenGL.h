#ifndef traktor_render_RenderSystemOpenGL_H
#define traktor_render_RenderSystemOpenGL_H

#include "Core/Heap/Ref.h"
#include "Render/IRenderSystem.h"
#include "Render/OpenGL/Platform.h"
#include "Render/OpenGL/ContextOpenGL.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_OPENGL_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

/*! \brief OpenGL render system.
 * \ingroup OGL
 *
 * OpenGL render system implementation.
 */
class T_DLLCLASS RenderSystemOpenGL : public IRenderSystem
{
	T_RTTI_CLASS(RenderSystemOpenGL)

public:
	RenderSystemOpenGL();

	virtual bool create();

	virtual void destroy();

	virtual int getDisplayModeCount() const;

	virtual DisplayMode* getDisplayMode(int index);

	virtual DisplayMode* getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual IRenderView* createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual IRenderView* createRenderView(void* windowHandle, const RenderViewCreateDesc& desc);

	virtual VertexBuffer* createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual IndexBuffer* createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual ISimpleTexture* createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual ICubeTexture* createCubeTexture(const CubeTextureCreateDesc& desc);

	virtual IVolumeTexture* createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual RenderTargetSet* createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual ProgramResource* compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate);

	virtual IProgram* createProgram(const ProgramResource* programResource);

private:
	friend class RenderViewOpenGL;

#if defined(_WIN32)

	HWND m_hWndShared;
	HWND m_hWnd;

	static LRESULT wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif

	Ref< ContextOpenGL > m_globalContext;
};

	}
}

#endif	// traktor_render_RenderSystemOpenGL_H
