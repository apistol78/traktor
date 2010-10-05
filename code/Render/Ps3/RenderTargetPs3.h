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
class StateCachePs3;

class T_DLLCLASS RenderTargetPs3 : public ITexture
{
	T_RTTI_CLASS;

public:
	RenderTargetPs3();

	bool create(MemoryHeap* memoryHeap, TileArea& tileArea, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual int getDepth() const;

	void bind(StateCachePs3& stateCache, int stage, const SamplerState& samplerState);

	void beginRender();

	void finishRender();

	const CellGcmTexture& getGcmColorTexture() {
		return m_colorTexture;
	}

	uint32_t getGcmSurfaceColorFormat() const {
		return m_colorSurfaceFormat;
	}

private:
	int32_t m_width;
	int32_t m_height;
	uint32_t m_colorSurfaceFormat;
	CellGcmTexture m_colorTexture;
	MemoryHeapObject* m_colorData;
	TileArea::TileInfo m_tileInfo;
	bool m_inRender;
};

	}
}

#endif	// traktor_render_RenderTargetPs3_H
