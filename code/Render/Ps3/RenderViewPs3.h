#ifndef traktor_render_RenderViewPs3_H
#define traktor_render_RenderViewPs3_H

#include <list>
#include "Render/IRenderView.h"
#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/ClearFpPs3.h"
#include "Render/Ps3/Resolve2xMSAA.h"
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
	RenderViewPs3(
		RenderSystemPs3* renderSystem,
		MemoryHeap* localMemoryHeap,
		MemoryHeap* mainMemoryHeap,
		TileArea& tileArea,
		TileArea& zcullArea
	);

	virtual ~RenderViewPs3();

	bool create(const RenderViewDefaultDesc& desc);

	virtual bool nextEvent(RenderEvent& outEvent);

	virtual void close();

	virtual bool reset(const RenderViewDefaultDesc& desc);

	virtual bool reset(int32_t width, int32_t height);

	virtual int getWidth() const;

	virtual int getHeight() const;

	virtual bool isActive() const;

	virtual bool isFullScreen() const;

	virtual bool setGamma(float gamma);

	virtual void setViewport(const Viewport& viewport);

	virtual Viewport getViewport();

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

private:
	struct RenderState
	{
		Viewport viewport;
		uint32_t width;
		uint32_t height;
		uint8_t antialias;
		uint8_t colorFormat;
		uint32_t colorOffset;
		uint32_t colorPitch;
		uint32_t depthFormat;
		uint32_t depthOffset;
		uint32_t depthPitch;
		uint32_t windowOffset;
		RenderTargetPs3* renderTarget;
		bool zcull;

		// Deferred clear.
		uint32_t clearMask;
		float clearColor[4];
		float clearDepth;
		int32_t clearStencil;
	};

	enum { FrameBufferCount = 2 };

	Ref< RenderSystemPs3 > m_renderSystem;

	// Heaps
	Ref< MemoryHeap > m_mainMemoryHeap;
	Ref< MemoryHeap > m_localMemoryHeap;

	// Command buffers.
	MemoryHeapObject* m_commandBuffers[FrameBufferCount];

	// Tiled region.
	TileArea& m_tileArea;
	TileArea::TileInfo m_colorTile[FrameBufferCount];
	TileArea::TileInfo m_depthTile;
#if defined(T_RENDER_PS3_USE_ZCULL)
	TileArea& m_zcullArea;
	TileArea::TileInfo m_zcullTile;
#endif

	// Current bound resources.
	Ref< VertexBufferPs3 > m_currentVertexBuffer;
	Ref< IndexBufferPs3 > m_currentIndexBuffer;
	Ref< ProgramPs3 > m_currentProgram;

	// Render view.
	int32_t m_width;
	int32_t m_height;
	Viewport m_viewport;
	float m_gamma;

	// Color buffers.
	MemoryHeapObject* m_colorObject;
	void* m_colorAddr[FrameBufferCount];
	uint32_t m_colorOffset[FrameBufferCount];
	uint32_t m_colorPitch;
	CellGcmTexture m_colorTexture;
	uint8_t m_targetSurfaceAntialias;
	CellGcmTexture m_targetTexture;
	MemoryHeapObject* m_targetData;
	TileArea::TileInfo m_targetTileInfo;

	// Depth buffers.
	MemoryHeapObject* m_depthObject;
	void* m_depthAddr;
	CellGcmTexture m_depthTexture;

	// State
	uint32_t m_frameCounter;
	uint32_t m_patchCounter;
	std::list< RenderState > m_renderTargetStack;
	bool m_renderTargetDirty;
	ClearFpPs3 m_clearFp;
	Resolve2xMSAA m_resolve2x;
	StateCachePs3 m_stateCache;
	float T_ALIGN16 m_targetSize[4];
	RenderViewStatistics m_statistics;

	void setCurrentRenderState();

	void clearImmediate();

	void blackOut();
};

	}
}

#endif	// traktor_render_RenderViewPs3_H
