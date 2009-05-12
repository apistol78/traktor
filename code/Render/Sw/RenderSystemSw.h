#ifndef traktor_render_RenderSystemSw_H
#define traktor_render_RenderSystemSw_H

#if defined(_WIN32)
#define _WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#endif
#include "Render/RenderSystem.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_SW_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace graphics
	{

class GraphicsSystem;

	}

	namespace render
	{

class Processor;

/*! \brief Software render system.
 * \ingroup SW
 *
 * Software render system implementation.
 */
class T_DLLCLASS RenderSystemSw : public RenderSystem
{
	T_RTTI_CLASS(RenderSystemSw)

public:
	RenderSystemSw();

	virtual bool create();

	virtual void destroy();

	virtual int getDisplayModeCount() const;

	virtual DisplayMode* getDisplayMode(int index);

	virtual DisplayMode* getCurrentDisplayMode();

	virtual bool handleMessages();

	virtual RenderView* createRenderView(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual RenderView* createRenderView(void* windowHandle, const RenderViewCreateDesc& desc);

	virtual VertexBuffer* createVertexBuffer(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize, bool dynamic);

	virtual IndexBuffer* createIndexBuffer(IndexType indexType, uint32_t bufferSize, bool dynamic);

	virtual SimpleTexture* createSimpleTexture(const SimpleTextureCreateDesc& desc);

	virtual CubeTexture* createCubeTexture(const CubeTextureCreateDesc& desc);

	virtual VolumeTexture* createVolumeTexture(const VolumeTextureCreateDesc& desc);

	virtual RenderTargetSet* createRenderTargetSet(const RenderTargetSetCreateDesc& desc);

	virtual ProgramResource* compileProgram(const ShaderGraph* shaderGraph, int optimize, bool validate);

	virtual Program* createProgram(const ProgramResource* programResource);

private:
	Ref< graphics::GraphicsSystem > m_graphicsSystem;
	Ref< Processor > m_processor;
#if defined(_WIN32)
	HWND m_hWnd;

	static LRESULT WINAPI wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif
};

	}
}

#endif	// traktor_render_RenderSystemSw_H
