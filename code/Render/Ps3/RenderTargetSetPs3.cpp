#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/RenderTargetSetPs3.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetSetPs3", RenderTargetSetPs3, RenderTargetSet)

RenderTargetSetPs3::RenderTargetSetPs3(
	TileArea& tileArea,
	TileArea& zcullArea,
	int32_t& counter
)
:	RenderTargetSet()
,	m_tileArea(tileArea)
#if defined(T_RENDER_PS3_USE_ZCULL)
,	m_zcullArea(zcullArea)
#endif
,	m_width(0)
,	m_height(0)
,	m_depthFormat(CELL_GCM_SURFACE_Z24S8)
,	m_depthData(0)
,	m_usingPrimaryDepthStencil(false)
,	m_counter(counter)
{
	std::memset(&m_depthTexture, 0, sizeof(m_depthTexture));
	++m_counter;
}

RenderTargetSetPs3::~RenderTargetSetPs3()
{
	destroy();
	--m_counter;
}

bool RenderTargetSetPs3::create(
	MemoryHeap* memoryHeap,
	const RenderTargetSetCreateDesc& desc
)
{
	bool allowZ16 = true;
	int32_t err;

	m_width = desc.width;
	m_height = desc.height;
	m_usingPrimaryDepthStencil = desc.usingPrimaryDepthStencil;

	m_renderTargets.resize(desc.count);
	for (int32_t i = 0; i < desc.count; ++i)
	{
		m_renderTargets[i] = new RenderTargetPs3(m_tileArea);
		if (!m_renderTargets[i]->create(memoryHeap, desc, desc.targets[i]))
			return false;

		// Z16 depth buffer not allowed with FP render targets.
		if (desc.targets[i].format >= TfR16G16B16A16F)
			allowZ16 = false;
	}

	if (desc.createDepthStencil)
	{
		m_depthFormat = (desc.ignoreStencil && allowZ16) ? CELL_GCM_SURFACE_Z16 : CELL_GCM_SURFACE_Z24S8;

		uint32_t depthWidth = m_width;
		uint32_t depthHeight = m_height;

		if (desc.multiSample > 1)
			depthWidth *= 2;

		m_depthTexture.format = CELL_GCM_TEXTURE_LN;
		
		if (desc.ignoreStencil && allowZ16)
			m_depthTexture.format |= CELL_GCM_TEXTURE_DEPTH16;
		else
			m_depthTexture.format |= CELL_GCM_TEXTURE_DEPTH24_D8;

		m_depthTexture.mipmap = 1;
		m_depthTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
		m_depthTexture.cubemap = 0;
		m_depthTexture.remap = 0;
		m_depthTexture.width = alignUp(depthWidth, 64);
		m_depthTexture.height = alignUp(depthHeight, 64);
		m_depthTexture.depth = 1;
		m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
		m_depthTexture.offset = 0;

		uint32_t depthFragmentSize = (desc.ignoreStencil && allowZ16) ? 2 : 4;
		if (desc.preferTiled)
			m_depthTexture.pitch = cellGcmGetTiledPitchSize(depthWidth * depthFragmentSize);
		else
			m_depthTexture.pitch = depthWidth * depthFragmentSize;

		// Tiled RT are immutable as we don't want to rebind tile area.
		uint32_t depthSize = alignUp(m_depthTexture.pitch * m_depthTexture.height, 65536);
		m_depthData = memoryHeap->alloc(depthSize, 65536, desc.preferTiled);

		if (desc.preferTiled)
		{
			if (m_tileArea.alloc(depthSize / 0x10000, 1, m_tileInfo))
			{
				uint32_t compression = CELL_GCM_COMPMODE_DISABLED;

				if (!(desc.ignoreStencil && allowZ16))
					compression = (desc.multiSample > 1) ?
						CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL :
						CELL_GCM_COMPMODE_Z32_SEPSTENCIL;

				err = cellGcmSetTileInfo(
					m_tileInfo.index,
					m_depthTexture.location,
					m_depthData->getOffset(),
					m_depthData->getSize(),
					m_depthTexture.pitch,
					compression,
					m_tileInfo.base,
					m_tileInfo.dramBank
				);
				if (err != CELL_OK)
					log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

				err = cellGcmBindTile(m_tileInfo.index);
				if (err != CELL_OK)
					log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;

#if defined(T_RENDER_PS3_USE_ZCULL)
				if (m_zcullArea.alloc(m_depthTexture.width * m_depthTexture.height, 4096, m_zcullInfo))
				{
					uint32_t zcullFormat = (desc.ignoreStencil && allowZ16) ? CELL_GCM_ZCULL_Z16 : CELL_GCM_ZCULL_Z24S8;
					err = cellGcmBindZcull(
						m_zcullInfo.index,
						m_depthTexture.offset,
						m_depthTexture.width,
						m_depthTexture.height,
						m_zcullInfo.base,
						zcullFormat,
						CELL_GCM_SURFACE_CENTER_1,
						CELL_GCM_ZCULL_GREATER,
						CELL_GCM_ZCULL_MSB,
						CELL_GCM_SCULL_SFUNC_ALWAYS,
						0,
						0
					);
					if (err != CELL_OK)
						log::error << L"Unable to bind ZCull (" << lookupGcmError(err) << L")" << Endl;
				}
#endif
			}
		}
	}
	else
	{
		m_depthTexture.width = 0;
		m_depthTexture.height = 0;
		m_depthTexture.depth = 0;
		m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
		m_depthTexture.pitch = 64;
		m_depthTexture.offset = 0;
		m_depthFormat = CELL_GCM_SURFACE_Z24S8;
	}

	return true;
}

void RenderTargetSetPs3::destroy()
{
#if defined(T_RENDER_PS3_USE_ZCULL)
	if (m_zcullInfo.index != ~0UL)
	{
		cellGcmUnbindZcull(m_zcullInfo.index);
		m_zcullArea.free(m_zcullInfo.index);
		m_zcullInfo.index = ~0UL;
	}
#endif

	if (m_tileInfo.index != ~0UL)
	{
		cellGcmUnbindTile(m_tileInfo.index);
		m_tileArea.free(m_tileInfo.index);
		m_tileInfo.index = ~0UL;
	}

	if (m_depthData)
	{
		m_depthData->free();
		m_depthData = 0;
	}

	for (RefArray< RenderTargetPs3 >::iterator i = m_renderTargets.begin(); i != m_renderTargets.end(); ++i)
		(*i)->destroy();
	
	m_renderTargets.clear();
}

int RenderTargetSetPs3::getWidth() const
{
	return m_width;
}

int RenderTargetSetPs3::getHeight() const
{
	return m_height;
}

ISimpleTexture* RenderTargetSetPs3::getColorTexture(int index) const
{
	return m_renderTargets[index];
}

ISimpleTexture* RenderTargetSetPs3::getDepthTexture() const
{
	return 0;
}

void RenderTargetSetPs3::swap(int index1, int index2)
{
	Ref< RenderTargetPs3 > rt1 = m_renderTargets[index1];
	Ref< RenderTargetPs3 > rt2 = m_renderTargets[index2];
	m_renderTargets[index1] = rt2;
	m_renderTargets[index2] = rt1;
}

void RenderTargetSetPs3::discard()
{
}

bool RenderTargetSetPs3::read(int index, void* buffer) const
{
	return false;
}

const CellGcmTexture& RenderTargetSetPs3::getGcmDepthTexture()
{
	if (m_depthData)
		m_depthTexture.offset = m_depthData->getOffset();

	return m_depthTexture;
}

	}
}
