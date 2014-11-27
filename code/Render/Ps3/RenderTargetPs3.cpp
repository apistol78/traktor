#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/Resolve2xMSAA.h"
#include "Render/Ps3/StateCachePs3.h"
#include "Render/Ps3/TypesPs3.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderTargetPs3", RenderTargetPs3, ITexture)

RenderTargetPs3::RenderTargetPs3(TileArea& tileArea)
:	m_tileArea(tileArea)
,	m_width(0)
,	m_height(0)
,	m_targetSurfaceFormat(0)
,	m_targetSurfaceAntialias(CELL_GCM_SURFACE_CENTER_1)
,	m_colorData(0)
,	m_targetData(0)
,	m_inRender(false)
{
}

bool RenderTargetPs3::create(MemoryHeap* memoryHeap, const RenderTargetSetCreateDesc& setDesc, const RenderTargetCreateDesc& desc)
{
	uint8_t byteSize;
	int32_t err;

	std::memset(&m_colorTexture, 0, sizeof(m_colorTexture));
	std::memset(&m_targetTexture, 0, sizeof(m_targetTexture));

	if (!getGcmSurfaceInfo(desc.format, m_colorTexture.format, m_targetSurfaceFormat, byteSize))
	{
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
		m_colorTexture.pitch = cellGcmGetTiledPitchSize(m_width * byteSize);
	else
		m_colorTexture.pitch = alignUp(m_width * byteSize, 64);

	uint32_t colorSize = alignUp(m_colorTexture.pitch * alignUp(m_colorTexture.height, 64), 65536);
	
	m_colorData = memoryHeap->alloc(colorSize, 65536, setDesc.preferTiled);
	if (!m_colorData)
	{
		log::error << L"Unable to allocate color buffer" << Endl;
		return false;
	}

	// Don't tile color buffer if RT multi-sampled; prioritize target buffer instead.
	if (setDesc.preferTiled && setDesc.multiSample > 1)
	{
		if (m_tileArea.alloc(colorSize / 0x10000, 1, m_colorTileInfo))
		{
			err = cellGcmSetTileInfo(
				m_colorTileInfo.index,
				CELL_GCM_LOCATION_LOCAL,
				m_colorData->getOffset(),
				m_colorData->getSize(),
				m_colorTexture.pitch,
				CELL_GCM_COMPMODE_DISABLED,
				m_colorTileInfo.base,
				m_colorTileInfo.dramBank
			);
			if (err != CELL_OK)
				log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

			err = cellGcmBindTile(m_colorTileInfo.index);
			if (err != CELL_OK)
				log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;
		}
	}

	if (setDesc.multiSample > 1)
	{
		int32_t targetWidth = m_width * 2;
		int32_t targetHeight = m_height;

		m_targetTexture.format = m_colorTexture.format | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
		m_targetTexture.mipmap = 1;
		m_targetTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
		m_targetTexture.cubemap = CELL_GCM_FALSE;
		m_targetTexture.remap =
			CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
			CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
			CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
			CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
			CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
			CELL_GCM_TEXTURE_REMAP_FROM_A;
		m_targetTexture.width = targetWidth;
		m_targetTexture.height = targetHeight;
		m_targetTexture.depth = 1;
		m_targetTexture.location = CELL_GCM_LOCATION_LOCAL;
		m_targetTexture.offset = 0;

		if (setDesc.preferTiled)
			m_targetTexture.pitch = cellGcmGetTiledPitchSize(targetWidth * byteSize);
		else
			m_targetTexture.pitch = targetWidth * byteSize;

		uint32_t targetColorSize = alignUp(m_targetTexture.pitch * alignUp(targetHeight, 64), 65536);

		m_targetData = memoryHeap->alloc(targetColorSize, 65536, setDesc.preferTiled);
		if (!m_targetData)
		{
			log::error << L"Unable to allocate target buffer" << Endl;
			return false;
		}

		if (setDesc.preferTiled)
		{
			if (m_tileArea.alloc(targetColorSize / 0x10000, 1, m_targetTileInfo))
			{
				err = cellGcmSetTileInfo(
					m_targetTileInfo.index,
					CELL_GCM_LOCATION_LOCAL,
					m_targetData->getOffset(),
					m_targetData->getSize(),
					m_targetTexture.pitch,
					CELL_GCM_COMPMODE_C32_2X1,
					m_targetTileInfo.base,
					m_targetTileInfo.dramBank
				);
				if (err != CELL_OK)
					log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

				err = cellGcmBindTile(m_targetTileInfo.index);
				if (err != CELL_OK)
					log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;
			}
		}

		m_targetSurfaceAntialias = CELL_GCM_SURFACE_DIAGONAL_CENTERED_2;
	}
	else
	{
		m_targetTexture = m_colorTexture;
	}

	return true;
}

void RenderTargetPs3::destroy()
{
	if (m_colorTileInfo.index != ~0UL)
	{
		cellGcmUnbindTile(m_colorTileInfo.index);
		m_tileArea.free(m_colorTileInfo.index);
		m_colorTileInfo.index = ~0UL;
	}

	if (m_targetTileInfo.index != ~0UL)
	{
		cellGcmUnbindTile(m_targetTileInfo.index);
		m_tileArea.free(m_targetTileInfo.index);
		m_targetTileInfo.index = ~0UL;
	}

	if (m_colorData)
	{
		m_colorData->free();
		m_colorData = 0;
	}

	if (m_targetData)
	{
		m_targetData->free();
		m_targetData = 0;
	}
}

ITexture* RenderTargetPs3::resolve()
{
	return this;
}

int RenderTargetPs3::getWidth() const
{
	return m_width;
}

int RenderTargetPs3::getHeight() const
{
	return m_height;
}

bool RenderTargetPs3::lock(int level, Lock& lock)
{
	return false;
}

void RenderTargetPs3::unlock(int level)
{
}

void RenderTargetPs3::bind(StateCachePs3& stateCache, int stage, const SamplerStateGCM& samplerState)
{
	T_ASSERT (!m_inRender);

	m_colorTexture.offset = m_colorData->getOffset();

	if (m_targetSurfaceFormat == CELL_GCM_SURFACE_B8 || m_targetSurfaceFormat == CELL_GCM_SURFACE_A8R8G8B8)
	{
		stateCache.setSamplerState(stage, samplerState);
		stateCache.setSamplerTexture(stage, &m_colorTexture, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
	}
	else	// FP targets.
	{
		SamplerStateGCM fpss;
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

	if (m_targetData)
		m_targetTexture.offset = m_targetData->getOffset();
	else
		m_targetTexture.offset = m_colorData->getOffset();

	m_inRender = true;
}

void RenderTargetPs3::finishRender(StateCachePs3& stateCache, Resolve2xMSAA& resolve)
{
	T_ASSERT (m_inRender == true);

	if (m_targetData)
	{
		resolve.resolve(
			stateCache,
			&m_colorTexture,
			&m_targetTexture
		);
	}

	m_inRender = false;
}

	}
}
