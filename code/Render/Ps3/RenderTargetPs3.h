#ifndef traktor_render_RenderTargetPs3_H
#define traktor_render_RenderTargetPs3_H

#include "Render/ITexture.h"
#include "Render/Ps3/TileArea.h"
#include "Render/Ps3/TypesPs3.h"

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

class MemoryHeap;
class MemoryHeapObject;
class Resolve2xMSAA;
class StateCachePs3;

class T_DLLCLASS RenderTargetPs3 : public ITexture
{
	T_RTTI_CLASS;

public:
	RenderTargetPs3(TileArea& tileArea);

	bool create(MemoryHeap* memoryHeap, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	void bind(StateCachePs3& stateCache, int stage, const SamplerState& samplerState);

	void beginRender();

	void finishRender(StateCachePs3& stateCache, Resolve2xMSAA& resolve);

	const CellGcmTexture& getGcmColorTexture() {
		return m_colorTexture;
	}

	const CellGcmTexture& getGcmTargetTexture() {
		return m_targetTexture;
	}

	uint8_t getGcmSurfaceColorFormat() const {
		return m_targetSurfaceFormat;
	}

	uint8_t getGcmSurfaceAntialias() const {
		return m_targetSurfaceAntialias;
	}

private:
	TileArea& m_tileArea;
	int32_t m_width;
	int32_t m_height;
	uint8_t m_targetSurfaceFormat;
	uint8_t m_targetSurfaceAntialias;
	CellGcmTexture m_colorTexture;
	CellGcmTexture m_targetTexture;
	MemoryHeapObject* m_colorData;
	MemoryHeapObject* m_targetData;
	TileArea::TileInfo m_colorTileInfo;
	TileArea::TileInfo m_targetTileInfo;
	bool m_inRender;
};

	}
}

#endif	// traktor_render_RenderTargetPs3_H
