#ifndef traktor_render_RenderViewPs3_H
#define traktor_render_RenderViewPs3_H

#include <list>
#include "Render/DisplayMode.h"
#include "Render/IRenderView.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/ClearFpPs3.h"
#include "Render/Ps3/StateCachePs3.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_RENDER_PS3_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace render
	{

class IndexBufferPs3;
class ProgramPs3;
class RenderSystemPs3;
class RenderTargetPs3;
class VertexBufferPs3;

class T_DLLCLASS RenderViewPs3 : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewPs3(RenderSystemPs3* renderSystem);

	virtual ~RenderViewPs3();

	bool create(const DisplayMode* displayMode, const RenderViewCreateDesc& desc);

	virtual void close();

	virtual void resize(int32_t width, int32_t height);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool getNativeAspectRatio(float& outAspectRatio) const;

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);
	
	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);
	
	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void setMSAAEnable(bool msaaEnable);

private:
	struct RenderState
	{
		Viewport viewport;
		uint32_t width;
		uint32_t height;
		uint32_t colorFormat;
		uint32_t colorOffset;
		uint32_t colorPitch;
		uint32_t depthOffset;
		uint32_t depthPitch;
		RenderTargetPs3* renderTarget;
	};

	Ref< RenderSystemPs3 > m_renderSystem;
	Ref< VertexBufferPs3 > m_currentVertexBuffer;
	Ref< IndexBufferPs3 > m_currentIndexBuffer;
	Ref< ProgramPs3 > m_currentProgram;
	int32_t m_width;
	int32_t m_height;
	Viewport m_viewport;
	LocalMemoryObject* m_colorObject;
	void* m_colorAddr[2];
	uint32_t m_colorOffset[2];
	uint32_t m_colorPitch;
	LocalMemoryObject* m_depthObject;
	void* m_depthAddr;
	CellGcmTexture m_depthTexture;
	uint32_t m_frameCounter;
	volatile uint32_t* m_frameSyncLabelData;
	std::list< RenderState > m_renderStateStack;
	ClearFpPs3 m_clearFp;
	StateCachePs3 m_stateCache;

	void setCurrentRenderState();
};

	}
}

#endif	// traktor_render_RenderViewPs3_H
