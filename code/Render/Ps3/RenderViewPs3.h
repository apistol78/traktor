#ifndef traktor_render_RenderViewPs3_H
#define traktor_render_RenderViewPs3_H

#include <list>
#include "Core/Memory/PoolAllocator.h"
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
class MemoryHeap;
class ProgramPs3;
class RenderSystemPs3;
class RenderTargetPs3;
class TileArea;
class VertexBufferPs3;

class T_DLLCLASS RenderViewPs3 : public IRenderView
{
	T_RTTI_CLASS;

public:
	RenderViewPs3(MemoryHeap* localMemoryHeap, TileArea& tileArea, RenderSystemPs3* renderSystem);

	virtual ~RenderViewPs3();

	bool create(const RenderViewDefaultDesc& desc);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual void resize(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

	virtual bool begin();

	virtual bool begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil);

	virtual void clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil);

	virtual void setVertexBuffer(VertexBuffer* vertexBuffer);
	
	virtual void setIndexBuffer(IndexBuffer* indexBuffer);

	virtual void setProgram(IProgram* program);
	
	virtual void draw(const Primitives& primitives);

	virtual void end();

	virtual void present();

	virtual void pushMarker(const char* const marker);

	virtual void popMarker();

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
		bool zcull;

		// Deferred clear.
		uint32_t clearMask;
		float clearColor[4];
		float clearDepth;
		int32_t clearStencil;
	};

	Ref< MemoryHeap > m_localMemoryHeap;
	TileArea& m_tileArea;
	Ref< RenderSystemPs3 > m_renderSystem;
	Ref< VertexBufferPs3 > m_currentVertexBuffer;
	Ref< IndexBufferPs3 > m_currentIndexBuffer;
	Ref< ProgramPs3 > m_currentProgram;
	int32_t m_width;
	int32_t m_height;
	Viewport m_viewport;
	MemoryHeapObject* m_colorObject;
	void* m_colorAddr[2];
	uint32_t m_colorOffset[2];
	uint32_t m_colorPitch;
	MemoryHeapObject* m_depthObject;
	void* m_depthAddr;
	CellGcmTexture m_depthTexture;
	uint32_t m_frameCounter;
	volatile uint32_t* m_frameSyncLabelData;
	std::list< RenderState > m_renderTargetStack;
	bool m_renderTargetDirty;
	ClearFpPs3 m_clearFp;
	StateCachePs3 m_stateCache;
	MemoryHeapObject* m_patchProgramObject;
	PoolAllocator m_patchProgramPool;
	float T_ALIGN16 m_targetSize[4];

	void setCurrentRenderState();

	void clearImmediate();
};

	}
}

#endif	// traktor_render_RenderViewPs3_H
