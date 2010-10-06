#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/StateCachePs3.h"
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetPs3", RenderTargetPs3, ITexture)

RenderTargetPs3::RenderTargetPs3()
:	m_width(0)
,	m_height(0)
,	m_colorSurfaceFormat(0)
,	m_colorData(0)
,	m_inRender(false)
{
	std::memset(&m_colorTexture, 0, sizeof(m_colorTexture));
}

bool RenderTargetPs3::create(MemoryHeap* memoryHeap, TileArea& tileArea, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	int byteSize;

	switch (desc.format)
	{
	case TfR8:
		//m_colorTexture.format = CELL_GCM_TEXTURE_B8;
		//m_colorSurfaceFormat = CELL_GCM_SURFACE_B8;
		//byteSize = 1;
		//break;

	case TfR8G8B8A8:
		m_colorTexture.format = CELL_GCM_TEXTURE_A8R8G8B8;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_A8R8G8B8;
		byteSize = 4;
		break;

	case TfR16G16B16A16F:
		m_colorTexture.format = CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_F_W16Z16Y16X16;
		byteSize = 4 * 2;
		break;

	case TfR32G32B32A32F:
		m_colorTexture.format = CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_F_W32Z32Y32X32;
		byteSize = 4 * 4;
		break;

	case TfR16F:
	case TfR32F:
		m_colorTexture.format = CELL_GCM_TEXTURE_X32_FLOAT;
		m_colorSurfaceFormat = CELL_GCM_SURFACE_F_X32;
		byteSize = 4;
		break;

	default:
		log::error << L"Unsupported render target surface format" << Endl;
		return false;
	}

	m_width = setDesc.width;
	m_height = setDesc.height;

	m_colorTexture.format |= CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
	m_colorTexture.mipmap = 1;
	m_colorTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
	m_colorTexture.cubemap = CELL_GCM_FALSE;
	m_colorTexture.remap =
		CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
		CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
		CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
		CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
		CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
		CELL_GCM_TEXTURE_REMAP_FROM_A;
	m_colorTexture.width = m_width;
	m_colorTexture.height = m_height;
	m_colorTexture.depth = 1;
	m_colorTexture.location = CELL_GCM_LOCATION_LOCAL;
	m_colorTexture.offset = 0;

	if (setDesc.preferTiled)
		m_colorTexture.pitch = cellGcmGetTiledPitchSize(alignUp(m_width, 64) * byteSize);
	else
		m_colorTexture.pitch = m_width * byteSize;

	uint32_t colorSize = setDesc.preferTiled ?
		m_colorTexture.pitch * alignUp(m_colorTexture.height, 64) :
		m_colorTexture.pitch * m_colorTexture.height;

	// Tiled RT are immutable as we don't want to rebind tile area.
	m_colorData = memoryHeap->alloc(colorSize, 4096, setDesc.preferTiled);

	if (setDesc.preferTiled)
	{
		if (tileArea.alloc(colorSize / 0x10000, 1, m_tileInfo))
		{
			cellGcmSetTileInfo(
				m_tileInfo.index,
				CELL_GCM_LOCATION_LOCAL,
				m_colorData->getOffset(),
				m_colorData->getSize(),
				m_colorTexture.pitch,
				CELL_GCM_COMPMODE_C32_2X1,
				m_tileInfo.base,
				m_tileInfo.dramBank
			);
			cellGcmBindTile(m_tileInfo.index);
		}
	}

	return true;
}

void RenderTargetPs3::destroy()
{
	if (m_tileInfo.index != ~0UL)
	{
		cellGcmUnbindTile(m_tileInfo.index);
		m_tileInfo.index = ~0UL;
	}

	if (m_colorData)
	{
		m_colorData->free();
		m_colorData = 0;
	}
}

int RenderTargetPs3::getWidth() const
{
	return m_width;
}

int RenderTargetPs3::getHeight() const
{
	return m_height;
}

int RenderTargetPs3::getDepth() const
{
	return 1;
}

void RenderTargetPs3::bind(StateCachePs3& stateCache, int stage, const SamplerState& samplerState)
{
	T_ASSERT (!m_inRender);

	m_colorTexture.offset = m_colorData->getOffset();

	if (m_colorSurfaceFormat == CELL_GCM_SURFACE_B8 || m_colorSurfaceFormat == CELL_GCM_SURFACE_A8R8G8B8)
	{
		stateCache.setSamplerState(stage, samplerState);
		stateCache.setSamplerTexture(stage, &m_colorTexture, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
	}
	else	// FP targets.
	{
		SamplerState fpss;
		fpss.minFilter = CELL_GCM_TEXTURE_NEAREST_NEAREST;
		fpss.magFilter = CELL_GCM_TEXTURE_NEAREST;
		fpss.wrapU = CELL_GCM_TEXTURE_CLAMP;
		fpss.wrapV = CELL_GCM_TEXTURE_CLAMP;
		fpss.wrapW = CELL_GCM_TEXTURE_CLAMP;

		stateCache.setSamplerState(stage, fpss);
		stateCache.setSamplerTexture(stage, &m_colorTexture, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
	}
}

void RenderTargetPs3::beginRender()
{
	T_ASSERT (m_inRender == false);

	m_colorTexture.offset = m_colorData->getOffset();
	m_inRender = true;
}

void RenderTargetPs3::finishRender()
{
	T_ASSERT (m_inRender == true);
	m_inRender = false;
}

	}
}
