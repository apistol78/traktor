#ifndef traktor_render_RenderViewXbox360_H
#define traktor_render_RenderViewXbox360_H

#include <list>
#include <map>
#include <stack>
#include "Core/Misc/ComRef.h"
#include "Render/IRenderView.h"
#include "Render/Dx9/Platform.h"

namespace traktor
{
	namespace render
	{

class RenderSystemXbox360;
class RenderTargetXbox360;
class RenderTargetPool;
class ProgramXbox360;
class VertexBufferDx9;
class IndexBufferDx9;

/*!
 * \ingroup Xbox360
 */
class RenderViewXbox360 : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewXbox360(
		const RenderViewDefaultDesc& createDesc,
		RenderSystemXbox360* renderSystem,
		IDirect3DDevice9* d3dDevice,
		RenderTargetPool* renderTargetPool,
		int width,
		int height,
		D3DFORMAT d3dPrimaryTargetFormat
	);

	virtual ~RenderViewXbox360();

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual void showCursor();

	virtual void hideCursor();

	virtual bool isCursorVisible() const;

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual SystemWindow getSystemWindow();

	virtual bool begin(EyeType eye);

	virtual bool begin(RenderTargetSet* renderTargetSet);

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget);

	virtual void clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives);

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

	virtual void getStatistics(RenderViewStatistics& outStatistics) const;

	virtual bool getBackBufferContent(void* buffer) const;

	IDirect3DDevice9* getD3DDevice() const { return m_d3dDevice; }

private:
	RenderViewDefaultDesc m_createDesc;
	Ref< RenderSystemXbox360 > m_renderSystem;
	ComRef< IDirect3DDevice9 > m_d3dDevice;
	Ref< RenderTargetPool > m_renderTargetPool;
	Ref< RenderTargetXbox360 > m_renderTargetPrimary;

	struct RenderState
	{
		D3DVIEWPORT9 d3dViewport;
		RenderTargetXbox360* renderTarget;
	};

	std::list< RenderState > m_renderStateStack;
	RenderState* m_currentRenderState;

	D3DVIEWPORT9 m_d3dViewport;

	Ref< VertexBufferDx9 > m_currentVertexBuffer;
	Ref< IndexBufferDx9 > m_currentIndexBuffer;
	Ref< ProgramXbox360 > m_currentProgram;

	DWORD m_defaultGPR[3];
};

	}
}

#endif	// traktor_render_RenderViewXbox360_H
