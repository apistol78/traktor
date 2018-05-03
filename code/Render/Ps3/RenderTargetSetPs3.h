/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_render_RenderTargetSetPs3_H
#define traktor_render_RenderTargetSetPs3_H

#include "Core/RefArray.h"
#include "Render/RenderTargetSet.h"
#include "Render/Types.h"
#include "Render/Ps3/TileArea.h"
#include "Render/Ps3/PlatformPs3.h"

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
class RenderTargetPs3;

class T_DLLCLASS RenderTargetSetPs3 : public RenderTargetSet
{
	T_RTTI_CLASS;

public:
	RenderTargetSetPs3(
		TileArea& tileArea,
		TileArea& zcullArea,
		int32_t& counter
	);

	virtual ~RenderTargetSetPs3();

	bool create(
		MemoryHeap* memoryHeap,
		const RenderTargetSetCreateDesc& desc
	);

	virtual void destroy();

	virtual int getWidth() const;
	
	virtual int getHeight() const;
	
	virtual ISimpleTexture* getColorTexture(int index) const;

	virtual ISimpleTexture* getDepthTexture() const;

	virtual void swap(int index1, int index2);

	virtual void discard();

	virtual bool isContentValid() const;

	virtual bool read(int index, void* buffer) const;

	const CellGcmTexture& getGcmDepthTexture();

	uint32_t getGcmDepthSurfaceFormat() const { return m_depthFormat; }

#if defined(T_RENDER_PS3_USE_ZCULL)
	bool getGcmZCull() const { return m_zcullInfo.index != ~0UL; }
#else
	bool getGcmZCull() const { return false; }
#endif

	uint32_t getRenderTargetCount() const { return m_renderTargets.size(); }

	RenderTargetPs3* getRenderTarget(int index) { return m_renderTargets[index]; }

	bool usingPrimaryDepthStencil() const { return m_usingPrimaryDepthStencil; }

	void setContentValid(bool contentValid) { m_contentValid = contentValid; }

private:
	TileArea& m_tileArea;
#if defined(T_RENDER_PS3_USE_ZCULL)
	TileArea& m_zcullArea;
#endif
	int32_t m_width;
	int32_t m_height;
	RefArray< RenderTargetPs3 > m_renderTargets;
	uint32_t m_depthFormat;
	CellGcmTexture m_depthTexture;
	MemoryHeapObject* m_depthData;
	TileArea::TileInfo m_tileInfo;
#if defined(T_RENDER_PS3_USE_ZCULL)
	TileArea::TileInfo m_zcullInfo;
#endif
	bool m_usingPrimaryDepthStencil;
	bool m_contentValid;
	int32_t& m_counter;
};

	}
}

#endif	// traktor_render_RenderTargetSetPs3_H
