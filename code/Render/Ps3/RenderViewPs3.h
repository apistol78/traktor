#pragma once

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

	virtual bool nextEvent(RenderEvent& outEvent) override final;

	virtual void close() override final;

	virtual bool reset(const RenderViewDefaultDesc& desc) override final;

	virtual bool reset(int32_t width, int32_t height) override final;

	virtual int getWidth() const override final;

	virtual int getHeight() const override final;

	virtual bool isActive() const override final;

	virtual bool isMinimized() const override final;

	virtual bool isFullScreen() const override final;

	virtual void showCursor() override final;

	virtual void hideCursor() override final;

	virtual bool isCursorVisible() const override final;

	virtual bool setGamma(float gamma) override final;

	virtual void setViewport(const Viewport& viewport) override final;

	virtual Viewport getViewport() override final;

	virtual SystemWindow getSystemWindow() override final;

	virtual bool begin(const Clear* clear) override final;

	virtual bool begin(RenderTargetSet* renderTargetSet, const Clear* clear) override final;

	virtual bool begin(RenderTargetSet* renderTargetSet, int32_t renderTarget, const Clear* clear) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives) override final;

	virtual void draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount) override final;

	virtual void compute(IProgram* program, const int32_t* workSize) override final;

	virtual void end() override final;

	virtual void present() override final;

	virtual void pushMarker(const char* const marker) override final;

	virtual void popMarker() override final;

	virtual void getStatistics(RenderViewStatistics& outStatistics) const override final;

	virtual bool getBackBufferContent(void* buffer) const override final;

private:
	struct RenderState
	{
		Viewport viewport;
		uint32_t width;
		uint32_t height;

		uint8_t antialias;
		uint8_t colorFormat;

		uint32_t colorOffset[4];
		uint32_t colorPitch[4];

		uint32_t depthFormat;
		uint32_t depthOffset;
		uint32_t depthPitch;
		uint32_t windowOffset;

		RenderTargetSetPs3* rts;
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
