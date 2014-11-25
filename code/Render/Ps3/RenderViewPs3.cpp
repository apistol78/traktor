#include <cstring>
#include "Core/Log/Log.h"
#include "Core/Misc/Align.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/ProgramPs3.h"
#include "Render/Ps3/RenderSystemPs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/RenderTargetSetPs3.h"
#include "Render/Ps3/RenderViewPs3.h"
#include "Render/Ps3/TileArea.h"
#include "Render/Ps3/VertexBufferPs3.h"

#define USE_DEBUG_DRAW		0
#define USE_TIME_MEASURE	0
#define LOG_PATCHED_COUNT	0

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_commandBufferSize = 512 * 1024;
const uint32_t c_reportZCullStats0 = 100;
const uint32_t c_reportZCullStats1 = 101;
const uint32_t c_reportTimeStamp0 = 102;
const uint32_t c_reportTimeStamp1 = 103;

const uint32_t c_labelFlipControl = 64;
const uint32_t c_labelFlipControlWait = 1;
const uint32_t c_labelFlipControlFlipIt = 2;

static volatile uint32_t* s_labelFlipControlData;

static uint32_t s_finishRef = 0;

void callbackUser(const uint32_t head)
{
	// We're running slow; issue flip immediately.
	*s_labelFlipControlData = c_labelFlipControlFlipIt;
}

void callbackVBlank(const uint32_t head)
{
	// We're running fast; issue flip at vblank.
	*s_labelFlipControlData = c_labelFlipControlFlipIt;
}

void callbackFlip(const uint32_t head)
{
	// Flipped; reset wait label.
	*s_labelFlipControlData = c_labelFlipControlWait;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewPs3", RenderViewPs3, IRenderView)

RenderViewPs3::RenderViewPs3(
	RenderSystemPs3* renderSystem,
	MemoryHeap* localMemoryHeap,
	MemoryHeap* mainMemoryHeap,
	TileArea& tileArea,
	TileArea& zcullArea
)
:	m_renderSystem(renderSystem)
,	m_localMemoryHeap(localMemoryHeap)
,	m_mainMemoryHeap(mainMemoryHeap)
,	m_tileArea(tileArea)
#if defined(T_RENDER_PS3_USE_ZCULL)
,	m_zcullArea(zcullArea)
#endif
,	m_width(0)
,	m_height(0)
,	m_gamma(1.0f)
,	m_colorObject(0)
,	m_colorPitch(0)
,	m_targetSurfaceAntialias(CELL_GCM_SURFACE_CENTER_1)
,	m_targetData(0)
,	m_depthObject(0)
,	m_depthAddr(0)
,	m_frameCounter(0)
,	m_patchCounter(0)
,	m_renderTargetDirty(false)
{
	std::memset(m_colorAddr, 0, sizeof(m_colorAddr));
	std::memset(m_colorOffset, 0, sizeof(m_colorOffset));
	std::memset(&m_depthTexture, 0, sizeof(m_depthTexture));
	std::memset(m_targetSize, 0, sizeof(m_targetSize));

	m_stateCache.reset();
}

RenderViewPs3::~RenderViewPs3()
{
}

bool RenderViewPs3::create(const RenderViewDefaultDesc& desc)
{
	// Create command buffers.
	for (uint32_t i = 0; i < sizeof_array(m_commandBuffers); ++i)
	{
		m_commandBuffers[i] = m_mainMemoryHeap->alloc(c_commandBufferSize, 0x100000, true);
		if (!m_commandBuffers[i])
			return false;
	}

	// Set our buffer.
	cellGcmSetCurrentBuffer((const uint32_t*)m_commandBuffers[0]->getPointer(), c_commandBufferSize);

	if (!reset(desc))
		return false;

	// Create helpers.
	m_clearFp.create(m_localMemoryHeap);
	m_resolve2x.create(m_localMemoryHeap);

	// Initialize flip control logic.
	s_labelFlipControlData = cellGcmGetLabelAddress(c_labelFlipControl);
	*s_labelFlipControlData = c_labelFlipControlWait;

	cellGcmSetUserHandler(callbackUser);
	cellGcmSetVBlankHandler(callbackVBlank);
	cellGcmSetFlipHandler(callbackFlip);

	return true;
}

bool RenderViewPs3::nextEvent(RenderEvent& outEvent)
{
	return false;
}

void RenderViewPs3::close()
{
	//blackOut();

	//if (m_zcullTile.index != ~0UL)
	//{
	//	cellGcmUnbindZcull(m_zcullTile.index);
	//	m_zcullArea.free(m_zcullTile.index);
	//	m_zcullTile.index = ~0UL;
	//}

	//if (m_depthTile.index != ~0UL)
	//{
	//	cellGcmUnbindTile(m_depthTile.index);
	//	m_tileArea.free(m_depthTile.index);
	//	m_depthTile.index = ~0UL;
	//}

	//if (m_targetTileInfo.index != ~0UL)
	//{
	//	cellGcmUnbindTile(m_targetTileInfo.index);
	//	m_tileArea.free(m_targetTileInfo.index);
	//	m_targetTileInfo.index = ~0UL;
	//}

	//for (int i = 0; i < sizeof_array(m_colorTile); ++i)
	//{
	//	if (m_colorTile[i].index != ~0UL)
	//	{
	//		cellGcmUnbindTile(m_colorTile[i].index);
	//		m_tileArea.free(m_colorTile[i].index);
	//		m_colorTile[i].index = ~0UL;
	//	}
	//}

	//if (m_depthObject)
	//{
	//	m_depthObject->free();
	//	m_depthObject = 0;
	//}

	//if (m_targetData)
	//{
	//	m_targetData->free();
	//	m_targetData = 0;
	//}

	//if (m_colorObject)
	//{
	//	m_colorObject->free();
	//	m_colorObject = 0;
	//}
}

bool RenderViewPs3::reset(const RenderViewDefaultDesc& desc)
{
	CellVideoOutState videoState;
	CellVideoOutConfiguration videoConfig;
	int32_t err;

	const ResolutionDesc* resolution = findResolutionDesc(
		desc.displayMode.width,
		desc.displayMode.height,
		desc.displayMode.stereoscopic
	);
	if (!resolution)
	{
		log::error << L"Create render view failed; no such resolution supported" << Endl;
		return false;
	}

	close();

	m_width = desc.displayMode.width;
	m_height = desc.displayMode.height;
	m_colorPitch = cellGcmGetTiledPitchSize(m_width * 4);

	std::memset(&videoConfig, 0, sizeof(CellVideoOutConfiguration));
	videoConfig.resolutionId = resolution->id;
	videoConfig.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videoConfig.aspect = CELL_VIDEO_OUT_ASPECT_AUTO;
	videoConfig.pitch = m_colorPitch;

	blackOut();

	err = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videoConfig, NULL, 0);
	if (err != CELL_OK)
	{
		log::error << L"Create render view failed; unable to configure video" << Endl;
		return false;
	}

	err = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (err != CELL_OK)
	{
		log::error << L"Create render view failed; unable get video state" << Endl;
		return false;
	}

	if (!desc.displayMode.stereoscopic)
	{
		cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);

		uint32_t colorSize = alignUp(m_colorPitch * alignUp(m_height, 64), 65536);
		m_colorObject = m_localMemoryHeap->alloc(sizeof_array(m_colorAddr) * colorSize, 65536, true);

		for (size_t i = 0; i < sizeof_array(m_colorAddr); i++)
		{
			m_colorAddr[i] = (uint8_t*)m_colorObject->getPointer() + (i * colorSize);
			m_colorOffset[i] = m_colorObject->getOffset() + (i * colorSize);

			// Only put color buffers in tiled memory if we're not multisampling.
			if (desc.multiSample <= 0)
			{
				if (m_tileArea.alloc(colorSize / 0x10000, 1, m_colorTile[i]))
				{
					err = cellGcmSetTileInfo(
						m_colorTile[i].index,
						CELL_GCM_LOCATION_LOCAL,
						m_colorOffset[i],
						colorSize,
						m_colorPitch,
						CELL_GCM_COMPMODE_DISABLED,
						m_colorTile[i].base,
						m_colorTile[i].dramBank
					);
					if (err != CELL_OK)
						log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

					err = cellGcmBindTile(m_colorTile[i].index);
					if (err != CELL_OK)
						log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;
				}
			}

			err = cellGcmSetDisplayBuffer(
				i,
				m_colorOffset[i],
				m_colorPitch,
				m_width,
				m_height
			);
			if (err != CELL_OK)
			{
				log::error << L"Create render view failed; unable to set display buffers" << Endl;
				return false;
			}
		}

		std::memset(&m_colorTexture, 0, sizeof(m_colorTexture));
		m_colorTexture.format = CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
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
		m_colorTexture.pitch = m_colorPitch;

		// Allocate multi-sampled target buffer.
		if (desc.multiSample > 1)
		{
			int32_t targetWidth = m_width * 2;
			int32_t targetHeight = m_height;

			std::memset(&m_targetTexture, 0, sizeof(m_targetTexture));
			m_targetTexture.format = CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
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
			m_targetTexture.pitch = cellGcmGetTiledPitchSize(targetWidth * 4);

			uint32_t targetColorSize = alignUp(m_targetTexture.pitch * alignUp(targetHeight, 64), 65536);

			m_targetData = m_localMemoryHeap->alloc(targetColorSize, 65536, true);
			if (!m_targetData)
			{
				log::error << L"Unable to allocate target buffer" << Endl;
				return false;
			}

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

			m_targetSurfaceAntialias = CELL_GCM_SURFACE_DIAGONAL_CENTERED_2;
		}

		// Allocate depth buffer.
		{
			int32_t depthWidth = m_width;
			int32_t depthHeight = m_height;

			if (desc.multiSample > 1)
				depthWidth *= 2;

			m_depthTexture.format = CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
			m_depthTexture.mipmap = 1;
			m_depthTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
			m_depthTexture.cubemap = 0;
			m_depthTexture.remap = 0;
			m_depthTexture.width = alignUp(depthWidth, 64);
			m_depthTexture.height = alignUp(depthHeight, 64);
			m_depthTexture.depth = 1;
			m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
			m_depthTexture.pitch = cellGcmGetTiledPitchSize(m_depthTexture.width * 4);
			m_depthTexture.offset = 0;

			uint32_t depthSize = alignUp(m_depthTexture.pitch * m_depthTexture.height, 65536);
			m_depthObject = m_localMemoryHeap->alloc(depthSize, 65536, true);

			m_depthAddr = m_depthObject->getPointer();
			m_depthTexture.offset = m_depthObject->getOffset();

			// Allocate tile area for depth buffer.
			if (m_tileArea.alloc(depthSize / 0x10000, 1, m_depthTile))
			{
				err = cellGcmSetTileInfo(
					m_depthTile.index,
					m_depthTexture.location,
					m_depthTexture.offset,
					depthSize,
					m_depthTexture.pitch,
					(desc.multiSample > 1) ?
						CELL_GCM_COMPMODE_Z32_SEPSTENCIL_DIAGONAL :
						CELL_GCM_COMPMODE_Z32_SEPSTENCIL,
					m_depthTile.base,
					m_depthTile.dramBank
				);
				if (err != CELL_OK)
					log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

				err = cellGcmBindTile(m_depthTile.index);
				if (err != CELL_OK)
					log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;
			}

#if defined(T_RENDER_PS3_USE_ZCULL)
			// Setup Z-cull binding.
			if (m_zcullArea.alloc(m_depthTexture.width * m_depthTexture.height, 4096, m_zcullTile))
			{
				err = cellGcmBindZcull(
					m_zcullTile.index,
					m_depthTexture.offset,
					m_depthTexture.width,
					m_depthTexture.height,
					0,
					CELL_GCM_ZCULL_Z24S8,
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
	else	// Setup stereoscopic frame buffers.
	{
		if (desc.multiSample > 1)
		{
			log::error << L"MSAA not permitted in stereoscopic views" << Endl;
			return false;
		}

		cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);

		// Allocate color buffer; 30 lines gap.
		uint32_t colorSize = alignUp(m_colorPitch * (m_height * 2 + 30), 65536);
		m_colorObject = m_localMemoryHeap->alloc(sizeof_array(m_colorAddr) * colorSize, 65536, true);

		for (size_t i = 0; i < sizeof_array(m_colorAddr); i++)
		{
			m_colorAddr[i] = (uint8_t*)m_colorObject->getPointer() + (i * colorSize);
			m_colorOffset[i] = m_colorObject->getOffset() + (i * colorSize);

			if (m_tileArea.alloc(colorSize / 0x10000, 1, m_colorTile[i]))
			{
				err = cellGcmSetTileInfo(
					m_colorTile[i].index,
					CELL_GCM_LOCATION_LOCAL,
					m_colorOffset[i],
					colorSize,
					m_colorPitch,
					CELL_GCM_COMPMODE_DISABLED,
					m_colorTile[i].base,
					m_colorTile[i].dramBank
				);
				if (err != CELL_OK)
					log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

				err = cellGcmBindTile(m_colorTile[i].index);
				if (err != CELL_OK)
					log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;
			}

			err = cellGcmSetDisplayBuffer(
				i,
				m_colorOffset[i],
				m_colorPitch,
				m_width,
				m_height * 2 + 30
			);
			if (err != CELL_OK)
			{
				log::error << L"Create render view failed; unable to set display buffers" << Endl;
				return false;
			}
		}

		// Allocate depth buffer; we share depth buffer for both eyes.
		{
			m_depthTexture.format = CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
			m_depthTexture.mipmap = 1;
			m_depthTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
			m_depthTexture.cubemap = 0;
			m_depthTexture.remap = 0;
			m_depthTexture.width = alignUp(m_width, 64);
			m_depthTexture.height = alignUp(m_height + 6, 64);	// 720+6 in order to be able use tiled memory.
			m_depthTexture.depth = 1;
			m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
			m_depthTexture.pitch = cellGcmGetTiledPitchSize(m_depthTexture.width * 4);
			m_depthTexture.offset = 0;

			uint32_t depthSize = alignUp(m_depthTexture.pitch * m_depthTexture.height, 65536);
			m_depthObject = m_localMemoryHeap->alloc(depthSize, 65536, true);

			m_depthAddr = m_depthObject->getPointer();
			m_depthTexture.offset = m_depthObject->getOffset();

			// Allocate tile area for depth buffer.
			if (m_tileArea.alloc(depthSize / 0x10000, 1, m_depthTile))
			{
				err = cellGcmSetTileInfo(
					m_depthTile.index,
					m_depthTexture.location,
					m_depthTexture.offset,
					depthSize,
					m_depthTexture.pitch,
					CELL_GCM_COMPMODE_Z32_SEPSTENCIL,
					m_depthTile.base,
					m_depthTile.dramBank
				);
				if (err != CELL_OK)
					log::error << L"Unable to set tile info (" << lookupGcmError(err) << L")" << Endl;

				err = cellGcmBindTile(m_depthTile.index);
				if (err != CELL_OK)
					log::error << L"Unable to bind tile (" << lookupGcmError(err) << L")" << Endl;
			}

#if defined(T_RENDER_PS3_USE_ZCULL)
			// Setup Z-cull binding.
			if (m_zcullArea.alloc(m_depthTexture.width * m_depthTexture.height, 4096, m_zcullTile))
			{
				err = cellGcmBindZcull(
					m_zcullTile.index,
					m_depthTexture.offset,
					m_depthTexture.width,
					m_depthTexture.height,
					0,
					CELL_GCM_ZCULL_Z24S8,
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

#if defined(T_RENDER_PS3_USE_ZCULL)
	if (m_zcullTile.index != ~0UL)
	{
		T_GCM_CALL(cellGcmSetZcullStatsEnable)(
			gCellGcmCurrentContext,
			CELL_GCM_TRUE
		);
	}
#endif

	setViewport(Viewport(0, 0, m_width, m_height, 0.0f, 1.0f));
	return true;
}

bool RenderViewPs3::reset(int32_t width, int32_t height)
{
	return false;
}

int RenderViewPs3::getWidth() const
{
	return m_width;
}

int RenderViewPs3::getHeight() const
{
	return m_height;
}

bool RenderViewPs3::isActive() const
{
	return true;
}

bool RenderViewPs3::isFullScreen() const
{
	return true;
}

bool RenderViewPs3::setGamma(float gamma)
{
	if (m_gamma == gamma)
		return true;

	int32_t res = cellVideoOutSetGamma(CELL_VIDEO_OUT_PRIMARY, clamp(gamma, 0.8f, 1.2f));
	if (res < 0)
		return false;

	m_gamma = gamma;
	return true;
}

void RenderViewPs3::setViewport(const Viewport& viewport)
{
	if (m_renderTargetStack.empty())
		m_viewport = viewport;
	else
	{
		m_renderTargetStack.back().viewport = viewport;
		if (!m_renderTargetDirty)
			m_stateCache.setViewport(viewport);
	}
}

Viewport RenderViewPs3::getViewport()
{
	if (m_renderTargetStack.empty())
		return m_viewport;
	else
		return m_renderTargetStack.back().viewport;
}

bool RenderViewPs3::begin(EyeType eye)
{
	// \hack Assume we're rendering Left then Right
	if (eye == EtCyclop || eye == EtLeft)
	{
		m_renderSystem->beginRendering();
#if USE_TIME_MEASURE
		T_GCM_CALL(cellGcmSetTimeStamp)(gCellGcmCurrentContext, c_reportTimeStamp0);
#endif

		// We need to compensate for 1-Z depth buffer arrangement by ensuring correct state is set.
		T_GCM_CALL(cellGcmSetDepthFunc)(gCellGcmCurrentContext, CELL_GCM_GREATER);
	}

	uint32_t frameIndex = m_frameCounter % sizeof_array(m_colorOffset);

	uint32_t eyeOffset = 0;
	uint32_t eyeWindowOffset = 0;
	if (eye == EtRight)
	{
		eyeOffset = 744 * m_colorPitch;
		eyeWindowOffset = 6;
	}

	if (!m_targetData)
	{
		RenderState rs =
		{
			m_viewport,
			m_width,
			m_height,
			CELL_GCM_SURFACE_CENTER_1,
			CELL_GCM_SURFACE_A8R8G8B8,
			m_colorOffset[frameIndex] + eyeOffset,
			m_colorPitch,
			CELL_GCM_SURFACE_Z24S8,
			m_depthTexture.offset,
			m_depthTexture.pitch,
			eyeWindowOffset,
			0,
			true,
			0
		};

		T_ASSERT (m_renderTargetStack.empty());
		m_renderTargetStack.push_back(rs);
	}
	else
	{
		m_targetTexture.offset = m_targetData->getOffset();

		RenderState rs =
		{
			m_viewport,
			m_width,
			m_height,
			m_targetSurfaceAntialias,
			CELL_GCM_SURFACE_A8R8G8B8,
			m_targetTexture.offset,
			m_targetTexture.pitch,
			CELL_GCM_SURFACE_Z24S8,
			m_depthTexture.offset,
			m_depthTexture.pitch,
			0,
			0,
			true,
			0
		};

		T_ASSERT (m_renderTargetStack.empty());
		m_renderTargetStack.push_back(rs);
	}

	m_renderTargetDirty = true;

#if defined(T_RENDER_PS3_USE_ZCULL)
	// Update Z-cull limits.
	int32_t	maxSlope = cellGcmGetReport(CELL_GCM_ZCULL_STATS, c_reportZCullStats0);
	int32_t	sumSlope = cellGcmGetReport(CELL_GCM_ZCULL_STATS1, c_reportZCullStats1);
	int32_t numTiles = maxSlope & 0xffff;
	maxSlope = (maxSlope & 0xFFFF0000) >> 16;
	int32_t avgSlope = numTiles ? sumSlope / numTiles : 0;

	int32_t moveForward = (avgSlope + maxSlope) / 2;
	int32_t pushBack = moveForward / 2;

	if (moveForward < 1)
		moveForward = 1;

	if (pushBack < 1)
		pushBack = 1;

	T_GCM_CALL(cellGcmSetZcullLimit)(
		gCellGcmCurrentContext,
		moveForward,
		pushBack
	);
	T_GCM_CALL(cellGcmSetClearReport)(gCellGcmCurrentContext, CELL_GCM_ZCULL_STATS);
#endif

	return true;
}

bool RenderViewPs3::begin(RenderTargetSet* renderTargetSet)
{
	return false;
}

bool RenderViewPs3::begin(RenderTargetSet* renderTargetSet, int renderTarget)
{
	T_ASSERT (!m_renderTargetStack.empty());

	RenderTargetSetPs3* rts = checked_type_cast< RenderTargetSetPs3* >(renderTargetSet);
	RenderTargetPs3* rt = rts->getRenderTarget(renderTarget);
	T_ASSERT (rt);

	rt->beginRender();

	RenderState rs =
	{
		Viewport(0, 0, rt->getWidth(), rt->getHeight(), 0.0f, 1.0f),
		rt->getWidth(),
		rt->getHeight(),
		rt->getGcmSurfaceAntialias(),
		rt->getGcmSurfaceColorFormat(),
		rt->getGcmTargetTexture().offset,
		rt->getGcmTargetTexture().pitch,
		rts->getGcmDepthSurfaceFormat(),
		rts->getGcmDepthTexture().offset,
		rts->getGcmDepthTexture().pitch,
		0,
		rt,
		rts->getGcmZCull(),
		0
	};

	if (rts->usingPrimaryDepthStencil())
	{
		T_ASSERT_M (rt->getWidth() == m_width && rt->getHeight() == m_height, L"Target dimension mismatch");
		T_ASSERT_M (rt->getGcmSurfaceAntialias() == m_targetSurfaceAntialias, L"Target multisampling mismatch");
		rs.depthFormat = CELL_GCM_SURFACE_Z24S8;
		rs.depthOffset = m_depthTexture.offset;
		rs.depthPitch = m_depthTexture.pitch;
#if defined(T_RENDER_PS3_USE_ZCULL)
		rs.zcull = true;
#else
		rs.zcull = false;
#endif
	}

	m_renderTargetStack.push_back(rs);
	m_renderTargetDirty = true;

	rts->setContentValid(true);

	return true;
}

void RenderViewPs3::clear(uint32_t clearMask, const Color4f* colors, float depth, int32_t stencil)
{
	T_ASSERT (!m_renderTargetStack.empty());

	RenderState& rs = m_renderTargetStack.back();
	rs.clearMask |= clearMask;
	if (clearMask & CfColor)
	{
		rs.clearColor[0] = colors[0].getRed();
		rs.clearColor[1] = colors[0].getGreen();
		rs.clearColor[2] = colors[0].getBlue();
		rs.clearColor[3] = colors[0].getAlpha();
	}
	if (clearMask & CfDepth)
		rs.clearDepth = depth;
	if (clearMask & CfStencil)
		rs.clearStencil = stencil;

	if (!m_renderTargetDirty)
		clearImmediate();
}

void RenderViewPs3::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives)
{
	const uint8_t c_mode[] =
	{
		CELL_GCM_PRIMITIVE_POINTS,
		CELL_GCM_PRIMITIVE_LINE_STRIP,
		CELL_GCM_PRIMITIVE_LINES,
		CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,
		CELL_GCM_PRIMITIVE_TRIANGLES
	};

	m_currentVertexBuffer = checked_type_cast< VertexBufferPs3* >(vertexBuffer);
	m_currentIndexBuffer = checked_type_cast< IndexBufferPs3* >(indexBuffer);
	m_currentProgram = checked_type_cast< ProgramPs3* >(program);

	if (m_renderTargetDirty)
		setCurrentRenderState();

	m_currentProgram->bind(m_stateCache, m_targetSize, m_frameCounter, m_patchCounter);
	m_currentVertexBuffer->bind(m_stateCache, m_currentProgram->getInputSignature());

	uint32_t count = 0;
	switch (primitives.type)
	{
	case PtPoints:
		count = primitives.count;
		break;

	case PtLineStrip:
		count = primitives.count + 1;
		break;

	case PtLines:
		count = primitives.count * 2;
		break;

	case PtTriangleStrip:
		count = primitives.count + 2;
		break;

	case PtTriangles:
		count = primitives.count * 3;
		break;
	}

	if (primitives.indexed)
	{
		T_ASSERT (m_currentIndexBuffer);

		uint8_t type = CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16;
		uint32_t offset = m_currentIndexBuffer->getOffset() + primitives.offset * 2;

		if (m_currentIndexBuffer->getIndexType() == ItUInt32)
		{
			type = CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32;
			offset += primitives.offset * 2;
		}
		
		T_GCM_CALL(cellGcmSetDrawIndexArray)(
			gCellGcmCurrentContext,
			c_mode[primitives.type],
			count,
			type,
			m_currentIndexBuffer->getLocation(),
			offset
		);
	}
	else
	{
		T_GCM_CALL(cellGcmSetDrawArrays)(
			gCellGcmCurrentContext,
			c_mode[primitives.type],
			primitives.offset,
			count
		);
	}

#if USE_DEBUG_DRAW
	// Synchronize RSX after each draw; help to find RSX crashes.
	cellGcmFinish(gCellGcmCurrentContext, ++s_finishRef);
#endif
}

void RenderViewPs3::draw(VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer, IProgram* program, const Primitives& primitives, uint32_t instanceCount)
{
}

void RenderViewPs3::end()
{
	T_ASSERT (!m_renderTargetStack.empty());

	RenderState& rs = m_renderTargetStack.back();
	RenderTargetPs3* rt = rs.renderTarget;
	
	m_renderTargetStack.pop_back();

	if (!m_renderTargetStack.empty())
	{
		T_ASSERT (rt);
		rt->finishRender(m_stateCache, m_resolve2x);
		m_renderTargetDirty = true;
	}
#if defined(T_RENDER_PS3_USE_ZCULL)
	else
	{
		T_GCM_CALL(cellGcmSetReport)(gCellGcmCurrentContext, CELL_GCM_ZCULL_STATS, c_reportZCullStats0);
		T_GCM_CALL(cellGcmSetReport)(gCellGcmCurrentContext, CELL_GCM_ZCULL_STATS1, c_reportZCullStats1);
	}
#endif
}

void RenderViewPs3::present()
{
	uint32_t frameIndex = m_frameCounter % sizeof_array(m_colorAddr);

	// Resolve primary MSAA into color buffer before flipping.
	if (m_targetData)
	{
		m_colorTexture.offset = m_colorOffset[frameIndex];
		m_resolve2x.resolve(
			m_stateCache,
			&m_colorTexture,
			&m_targetTexture
		);
	}

	while (cellGcmGetFlipStatus() != 0)
		sys_timer_usleep(300);

	cellGcmResetFlipStatus();

	cellGcmSetFlipWithWaitLabel(gCellGcmCurrentContext, frameIndex, c_labelFlipControl, c_labelFlipControlFlipIt);
	//cellGcmSetWriteCommandLabel(c_labelFlipControl, c_labelFlipControlWait);
	//cellGcmSetWaitFlip();
	cellGcmFlush(gCellGcmCurrentContext);

	/*
	T_GCM_CALL(cellGcmSetWaitFlip)(gCellGcmCurrentContext);
	if (cellGcmSetFlip(gCellGcmCurrentContext, frameIndex) != CELL_OK)
		return;
	T_GCM_CALL(cellGcmFlush)(gCellGcmCurrentContext);
	*/

#if USE_TIME_MEASURE
	T_GCM_CALL(cellGcmSetTimeStamp)(gCellGcmCurrentContext, c_reportTimeStamp1);
#endif

	++m_frameCounter;

	frameIndex = m_frameCounter % sizeof_array(m_colorAddr);

	T_GCM_CALL(cellGcmSetJumpCommand)(gCellGcmCurrentContext, m_commandBuffers[frameIndex]->getOffset());
	cellGcmSetCurrentBuffer((const uint32_t*)m_commandBuffers[frameIndex]->getPointer(), c_commandBufferSize);

	volatile CellGcmControl* control = cellGcmGetControlRegister();
	volatile uint32_t get = (volatile uint32_t)control->get;

	while ((get >= m_commandBuffers[frameIndex]->getOffset() && get < (m_commandBuffers[frameIndex]->getOffset() + c_commandBufferSize)) || (get < 0x1000))
	{
		sys_timer_usleep(100);
		get = (volatile uint32_t)control->get;
	}

	// RSX context are reinitialized after each flip thus we need to reset our state cache.
	m_stateCache.reset();

	m_renderSystem->endRendering();

#if USE_TIME_MEASURE
	// Read timers to measure performance.
	uint64_t timeEnd = cellGcmGetTimeStamp(c_reportTimeStamp0);
	uint64_t timeStart = cellGcmGetTimeStamp(c_reportTimeStamp1);
	if (timeEnd > timeStart)
		m_statistics.duration = double(timeEnd - timeStart) / 1e9;
	else
		m_statistics.duration = double(timeStart - timeEnd) / 1e9;
#endif

#if LOG_PATCHED_COUNT
	if (m_patchCounter > 0)
		log::debug << m_patchCounter << L" fragment shader(s) patched" << Endl;
#endif
	m_patchCounter = 0;
}

void RenderViewPs3::pushMarker(const char* const marker)
{
	cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, marker ? marker : "<Unnamed>");
}

void RenderViewPs3::popMarker()
{
	cellGcmSetPerfMonPopMarker(gCellGcmCurrentContext);
}

void RenderViewPs3::getStatistics(RenderViewStatistics& outStatistics) const
{
	outStatistics = m_statistics;
}

void RenderViewPs3::setCurrentRenderState()
{
	T_ASSERT (m_renderTargetDirty);

	const RenderState& rs = m_renderTargetStack.back();

	CellGcmSurface sf;
	sf.type = CELL_GCM_SURFACE_PITCH;
	sf.antialias = rs.antialias;
	sf.colorFormat = rs.colorFormat;
	sf.colorTarget= CELL_GCM_SURFACE_TARGET_0;
	sf.colorLocation[0]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[0] = rs.colorOffset;
	sf.colorPitch[0] = rs.colorPitch;
	sf.colorLocation[1]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[1] = 0;
	sf.colorPitch[1] = 64;
	sf.colorLocation[2]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[2] = 0;
	sf.colorPitch[2] = 64;
	sf.colorLocation[3]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[3] = 0;
	sf.colorPitch[3] = 64;
	sf.depthFormat = rs.depthFormat;
	sf.depthLocation = CELL_GCM_LOCATION_LOCAL;
	sf.depthOffset = rs.depthOffset;
	sf.depthPitch = rs.depthPitch;
	sf.width = rs.width;
	sf.height = rs.height;
	sf.x = 0;
	sf.y = rs.windowOffset;

	T_GCM_CALL(cellGcmSetSurfaceWindow)(
		gCellGcmCurrentContext,
		&sf,
		CELL_GCM_WINDOW_ORIGIN_TOP,
		CELL_GCM_WINDOW_PIXEL_CENTER_HALF
	);

	if (rs.antialias != CELL_GCM_SURFACE_CENTER_1)
	{
		T_GCM_CALL(cellGcmSetAntiAliasingControl)(
			gCellGcmCurrentContext,
			CELL_GCM_TRUE,
			CELL_GCM_FALSE,
			CELL_GCM_FALSE,
			0xffff
		);
	}
	else
	{
		T_GCM_CALL(cellGcmSetAntiAliasingControl)(
			gCellGcmCurrentContext,
			CELL_GCM_FALSE,
			CELL_GCM_FALSE,
			CELL_GCM_FALSE,
			0xffff
		);
	}

#if defined(T_RENDER_PS3_USE_ZCULL)
	if (rs.zcull && rs.depthOffset)
	{
		T_GCM_CALL(cellGcmSetZcullEnable)(
			gCellGcmCurrentContext,
			CELL_GCM_TRUE,
			CELL_GCM_TRUE
		);
	}
	else
#endif
	{
		T_GCM_CALL(cellGcmSetZcullEnable)(
			gCellGcmCurrentContext,
			CELL_GCM_FALSE,
			CELL_GCM_FALSE
		);
	}

	// Set target size vertex shader constant.
	m_targetSize[0] = rs.width;
	m_targetSize[1] = rs.height;

	// Reset state cache; apparently render states somehow get automatically altered when changing render target.
	m_stateCache.setInFp32Mode((rs.colorFormat == CELL_GCM_SURFACE_F_W32Z32Y32X32 || rs.colorFormat == CELL_GCM_SURFACE_F_X32));
	m_stateCache.setViewport(rs.viewport);
	m_stateCache.setVertexShaderConstant(0, 1, m_targetSize);

	// Ensure target is cleared.
	clearImmediate();

	m_renderTargetDirty = false;
}

void RenderViewPs3::clearImmediate()
{
	RenderState& rs = m_renderTargetStack.back();
	uint32_t gcmClearMask = 0;

	if (rs.clearMask & CfColor)
	{
		if (
			rs.colorFormat == CELL_GCM_SURFACE_A8R8G8B8 ||
			rs.colorFormat == CELL_GCM_SURFACE_B8
		)
		{
			gcmClearMask |= CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A;

			uint32_t clearColor =
				(uint32_t(rs.clearColor[0] * 255.0f) << 16) |
				(uint32_t(rs.clearColor[1] * 255.0f) << 8) |
				(uint32_t(rs.clearColor[2] * 255.0f) << 0) |
				(uint32_t(rs.clearColor[3] * 255.0f) << 24);

			T_GCM_CALL(cellGcmSetClearColor)(gCellGcmCurrentContext, clearColor);
		}
		else
		{
			m_clearFp.clear(m_stateCache, rs.clearColor);
		}
	}

	if (rs.clearMask & (CfDepth | CfStencil))
	{
		if (rs.depthOffset)
		{
			float clearDepth = 1.0f - rs.clearDepth;	// "1-Z" depth buffer.
			gcmClearMask |= CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_S;
			T_GCM_CALL(cellGcmSetClearDepthStencil)(
				gCellGcmCurrentContext,
				(uint32_t(clearDepth * 0xffffff) << 8) | (rs.clearStencil & 0xff)
			);
		}
#if defined(T_RENDER_PS3_USE_ZCULL)
		//if (rs.zcull)
		//	T_GCM_CALL(cellGcmSetInvalidateZcull)(gCellGcmCurrentContext);
#endif
	}

	if (gcmClearMask)
		T_GCM_CALL(cellGcmSetClearSurface)(gCellGcmCurrentContext, gcmClearMask);

	rs.clearMask = 0;
}

void RenderViewPs3::blackOut()
{
	cellGcmSetWaitFlip(gCellGcmCurrentContext);
	cellGcmFinish(gCellGcmCurrentContext, ++s_finishRef);
}

	}
}
