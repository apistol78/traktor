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
#define USE_TIME_MEASURE	1

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_reportZCullStats0 = 100;
const uint32_t c_reportZCullStats1 = 101;
const uint32_t c_reportTimeStamp0 = 102;
const uint32_t c_reportTimeStamp1 = 103;
const uint8_t c_frameSyncLabelId = 128;
const uint8_t c_waitLabelId = 129;

uint32_t incrementLabel(uint32_t label)
{
	return (++label != 0) ? label : 1;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewPs3", RenderViewPs3, IRenderView)

RenderViewPs3::RenderViewPs3(
	MemoryHeap* localMemoryHeap,
	TileArea& tileArea,
	TileArea& zcullArea,
	RenderSystemPs3* renderSystem
)
:	m_localMemoryHeap(localMemoryHeap)
,	m_tileArea(tileArea)
,	m_zcullArea(zcullArea)
,	m_renderSystem(renderSystem)
,	m_width(0)
,	m_height(0)
,	m_colorObject(0)
,	m_colorPitch(0)
,	m_depthObject(0)
,	m_depthAddr(0)
,	m_frameCounter(1)
,	m_patchCounter(0)
,	m_frameSyncLabelData(0)
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
	if (!reset(desc))
		return false;

	// Create FP target clear helper.
	m_clearFp.create(m_localMemoryHeap);

	// Create frame synchronization labels.
	m_frameSyncLabelData = cellGcmGetLabelAddress(c_frameSyncLabelId);
	*m_frameSyncLabelData = m_frameCounter;

	volatile uint32_t* waitLabelData = cellGcmGetLabelAddress(c_waitLabelId);
	*waitLabelData = 0;

	return true;
}

void RenderViewPs3::close()
{
	if (m_zcullTile.index != ~0UL)
	{
		cellGcmUnbindZcull(m_zcullTile.index);
		m_zcullArea.free(m_zcullTile.index);
		m_zcullTile.index = ~0UL;
	}

	if (m_depthTile.index != ~0UL)
	{
		cellGcmUnbindTile(m_depthTile.index);
		m_tileArea.free(m_depthTile.index);
		m_depthTile.index = ~0UL;
	}

	for (int i = 0; i < sizeof_array(m_colorTile); ++i)
	{
		cellGcmUnbindTile(m_colorTile[i].index);
		m_tileArea.free(m_colorTile[i].index);
		m_colorTile[i].index = ~0UL;
	}

	if (m_depthObject)
	{
		m_depthObject->free();
		m_depthObject = 0;
	}

	if (m_colorObject)
	{
		m_colorObject->free();
		m_colorObject = 0;
	}
}

bool RenderViewPs3::reset(const RenderViewDefaultDesc& desc)
{
	CellVideoOutState videoState;
	CellVideoOutConfiguration videoConfig;
	int32_t ret;

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
	m_colorPitch = cellGcmGetTiledPitchSize(alignUp(m_width, 64) * 4);

	std::memset(&videoConfig, 0, sizeof(CellVideoOutConfiguration));
	videoConfig.resolutionId = resolution->id;
	videoConfig.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videoConfig.aspect = CELL_VIDEO_OUT_ASPECT_AUTO;
	videoConfig.pitch = m_colorPitch;

	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videoConfig, NULL, 0);
	if (ret != CELL_OK)
	{
		log::error << L"Create render view failed; unable to configure video" << Endl;
		return false;
	}

	ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (ret != CELL_OK)
	{
		log::error << L"Create render view failed; unable get video state" << Endl;
		return false;
	}

	if (!desc.displayMode.stereoscopic)
	{
		if (desc.waitVBlank)
			cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);
		else
			cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);

		uint32_t colorSize = m_colorPitch * alignUp(m_height, 64);
		m_colorObject = m_localMemoryHeap->alloc(sizeof_array(m_colorAddr) * colorSize, 4096, true);

		for (size_t i = 0; i < sizeof_array(m_colorAddr); i++)
		{
			m_colorAddr[i] = (uint8_t*)m_colorObject->getPointer() + (i * colorSize);
			m_colorOffset[i] = m_colorObject->getOffset() + (i * colorSize);

			if (m_tileArea.alloc(colorSize / 0x10000, 1, m_colorTile[i]))
			{
				cellGcmSetTileInfo(
					m_colorTile[i].index,
					CELL_GCM_LOCATION_LOCAL,
					m_colorOffset[i],
					colorSize,
					m_colorPitch,
					CELL_GCM_COMPMODE_C32_2X1,
					m_colorTile[i].base,
					m_colorTile[i].dramBank
				);
				cellGcmBindTile(m_colorTile[i].index);
			}

			ret = cellGcmSetDisplayBuffer(
				i,
				m_colorOffset[i],
				m_colorPitch,
				m_width,
				m_height
			);
			if (ret != CELL_OK)
			{
				log::error << L"Create render view failed; unable to set display buffers" << Endl;
				return false;
			}
		}

		// Allocate depth buffer.
		{
			m_depthTexture.format = CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
			m_depthTexture.mipmap = 1;
			m_depthTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
			m_depthTexture.cubemap = 0;
			m_depthTexture.remap = 0;
			m_depthTexture.width = alignUp(m_width, 64);
			m_depthTexture.height = alignUp(m_height, 64);
			m_depthTexture.depth = 1;
			m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
			m_depthTexture.pitch = cellGcmGetTiledPitchSize(m_depthTexture.width * 4);
			m_depthTexture.offset = 0;

			uint32_t depthSize = m_depthTexture.pitch * m_depthTexture.height;
			m_depthObject = m_localMemoryHeap->alloc(depthSize, 4096, true);

			m_depthAddr = m_depthObject->getPointer();
			m_depthTexture.offset = m_depthObject->getOffset();

			// Allocate tile area for depth buffer.
			if (m_tileArea.alloc(depthSize / 0x10000, 1, m_depthTile))
			{
				cellGcmSetTileInfo(
					m_depthTile.index,
					m_depthTexture.location,
					m_depthTexture.offset,
					depthSize,
					m_depthTexture.pitch,
					CELL_GCM_COMPMODE_Z32_SEPSTENCIL,
					m_depthTile.base,
					m_depthTile.dramBank
				);
				cellGcmBindTile(m_depthTile.index);
			}

			// Setup Z-cull binding.
			if (m_zcullArea.alloc(m_depthTexture.width * m_depthTexture.height, 4096, m_zcullTile))
			{
				cellGcmBindZcull(
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
			}
		}
	}
	else	// Setup stereoscopic frame buffers.
	{
		cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);

		// Allocate color buffer; 30 lines gap.
		uint32_t colorSize = m_colorPitch * alignUp(m_height * 2 + 30, 64);
		m_colorObject = m_localMemoryHeap->alloc(sizeof_array(m_colorAddr) * colorSize, 4096, true);

		for (size_t i = 0; i < sizeof_array(m_colorAddr); i++)
		{
			m_colorAddr[i] = (uint8_t*)m_colorObject->getPointer() + (i * colorSize);
			m_colorOffset[i] = m_colorObject->getOffset() + (i * colorSize);

			if (m_tileArea.alloc(colorSize / 0x10000, 1, m_colorTile[i]))
			{
				cellGcmSetTileInfo(
					m_colorTile[i].index,
					CELL_GCM_LOCATION_LOCAL,
					m_colorOffset[i],
					colorSize,
					m_colorPitch,
					CELL_GCM_COMPMODE_C32_2X1,
					m_colorTile[i].base,
					m_colorTile[i].dramBank
				);
				cellGcmBindTile(m_colorTile[i].index);
			}

			ret = cellGcmSetDisplayBuffer(
				i,
				m_colorOffset[i],
				m_colorPitch,
				m_width,
				m_height * 2 + 30
			);
			if (ret != CELL_OK)
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

			uint32_t depthSize = m_depthTexture.pitch * m_depthTexture.height;
			m_depthObject = m_localMemoryHeap->alloc(depthSize, 4096, true);

			m_depthAddr = m_depthObject->getPointer();
			m_depthTexture.offset = m_depthObject->getOffset();

			// Allocate tile area for depth buffer.
			if (m_tileArea.alloc(depthSize / 0x10000, 1, m_depthTile))
			{
				cellGcmSetTileInfo(
					m_depthTile.index,
					m_depthTexture.location,
					m_depthTexture.offset,
					depthSize,
					m_depthTexture.pitch,
					CELL_GCM_COMPMODE_Z32_SEPSTENCIL,
					m_depthTile.base,
					m_depthTile.dramBank
				);
				cellGcmBindTile(m_depthTile.index);
			}

			// Setup Z-cull binding.
			if (m_zcullArea.alloc(m_depthTexture.width * m_depthTexture.height, 4096, m_zcullTile))
			{
				cellGcmBindZcull(
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
			}
		}
	}

	if (m_zcullTile.index != ~0UL)
	{
		T_GCM_CALL(cellGcmSetZcullStatsEnable)(
			gCellGcmCurrentContext,
			CELL_GCM_TRUE
		);
	}

	setViewport(Viewport(0, 0, m_width, m_height, 0.0f, 1.0f));
	return true;
}

void RenderViewPs3::resize(int width, int height)
{
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
	}

	uint32_t frameIndex = m_frameCounter % sizeof_array(m_colorOffset);

	uint32_t eyeOffset = 0;
	uint32_t eyeWindowOffset = 0;
	if (eye == EtRight)
	{
		eyeOffset = 744 * m_colorPitch;
		eyeWindowOffset = 6;
	}

	RenderState rs =
	{
		m_viewport,
		m_width,
		m_height,
		CELL_GCM_SURFACE_A8R8G8B8,
		m_colorOffset[frameIndex] + eyeOffset,
		m_colorPitch,
		m_depthTexture.offset,
		m_depthTexture.pitch,
		eyeWindowOffset,
		0,
		true,
		0
	};

	T_ASSERT (m_renderTargetStack.empty());
	m_renderTargetStack.push_back(rs);
	m_renderTargetDirty = true;

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

	return true;
}

bool RenderViewPs3::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
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
		rt->getGcmSurfaceColorFormat(),
		rt->getGcmColorTexture().offset,
		rt->getGcmColorTexture().pitch,
		rts->getGcmDepthTexture().offset,
		rts->getGcmDepthTexture().pitch,
		0,
		rt,
		rts->getGcmZCull(),
		0
	};

	if (keepDepthStencil)
	{
		rs.depthOffset = m_renderTargetStack.back().depthOffset;
		rs.depthPitch = m_renderTargetStack.back().depthPitch;
		rs.zcull = m_renderTargetStack.back().zcull;
	}

	m_renderTargetStack.push_back(rs);
	m_renderTargetDirty = true;

	rts->setContentValid(true);

	return true;
}

void RenderViewPs3::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	T_ASSERT (!m_renderTargetStack.empty());

	RenderState& rs = m_renderTargetStack.back();
	rs.clearMask |= clearMask;
	if (clearMask & CfColor)
	{
		rs.clearColor[0] = color[0];
		rs.clearColor[1] = color[1];
		rs.clearColor[2] = color[2];
		rs.clearColor[3] = color[3];
	}
	if (clearMask & CfDepth)
		rs.clearDepth = depth;
	if (clearMask & CfStencil)
		rs.clearStencil = stencil;

	if (!m_renderTargetDirty)
		clearImmediate();
}

void RenderViewPs3::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferPs3* >(vertexBuffer);
}

void RenderViewPs3::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_currentIndexBuffer = checked_type_cast< IndexBufferPs3* >(indexBuffer);
}

void RenderViewPs3::setProgram(IProgram* program)
{
	m_currentProgram = checked_type_cast< ProgramPs3* >(program);
}

void RenderViewPs3::draw(const Primitives& primitives)
{
	const uint8_t c_mode[] =
	{
		CELL_GCM_PRIMITIVE_POINTS,
		CELL_GCM_PRIMITIVE_LINE_STRIP,
		CELL_GCM_PRIMITIVE_LINES,
		CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,
		CELL_GCM_PRIMITIVE_TRIANGLES
	};

	T_ASSERT (m_currentVertexBuffer);
	T_ASSERT (m_currentProgram);

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
	static uint32_t s_finishRef = 0; s_finishRef++;
	cellGcmFinish(gCellGcmCurrentContext, s_finishRef);
#endif
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
		rt->finishRender();
		m_renderTargetDirty = true;
	}
	else
	{
		T_GCM_CALL(cellGcmSetReport)(gCellGcmCurrentContext, CELL_GCM_ZCULL_STATS, c_reportZCullStats0);
		T_GCM_CALL(cellGcmSetReport)(gCellGcmCurrentContext, CELL_GCM_ZCULL_STATS1, c_reportZCullStats1);
	}
}

void RenderViewPs3::present()
{
	T_GCM_CALL(cellGcmSetWriteBackEndLabel)(gCellGcmCurrentContext, c_frameSyncLabelId, m_frameCounter);
	T_GCM_CALL(cellGcmFlush)(gCellGcmCurrentContext);

#if USE_TIME_MEASURE
	T_GCM_CALL(cellGcmSetTimeStamp)(gCellGcmCurrentContext, c_reportTimeStamp1);
#endif

	if (cellGcmSetFlip(gCellGcmCurrentContext, m_frameCounter % sizeof_array(m_colorAddr)) != CELL_OK)
	{
		log::error << L"Failed to present, unable to issue flip" << Endl;
		return;
	}
	T_GCM_CALL(cellGcmSetWaitFlip)(gCellGcmCurrentContext);

	while (*m_frameSyncLabelData + 1 < m_frameCounter)
		sys_timer_usleep(100);

	m_frameCounter = incrementLabel(m_frameCounter);

	m_renderSystem->endRendering();

	// RSX context are reinitialized after each flip thus we need to reset our state cache.
	m_stateCache.reset();

#if USE_TIME_MEASURE
	// Read timers to measure performance.
	uint64_t timeEnd = cellGcmGetTimeStamp(c_reportTimeStamp0);
	uint64_t timeStart = cellGcmGetTimeStamp(c_reportTimeStamp1);
	if (timeEnd > timeStart)
		m_statistics.duration = double(timeEnd - timeStart) / 1e9;
	else
		m_statistics.duration = double(timeStart - timeEnd) / 1e9;
#endif

#if defined(_DEBUG)
	if (m_patchCounter > 0)
		log::debug << m_patchCounter << L" fragment shader(s) patched" << Endl;
#endif
	m_patchCounter = 0;
}

void RenderViewPs3::pushMarker(const char* const marker)
{
	cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, marker);
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
	sf.depthFormat = CELL_GCM_SURFACE_Z24S8;
	sf.depthLocation = CELL_GCM_LOCATION_LOCAL;
	sf.depthOffset = rs.depthOffset;
	sf.depthPitch = rs.depthPitch;
	sf.type = CELL_GCM_SURFACE_PITCH;
	sf.antialias = CELL_GCM_SURFACE_CENTER_1;
	sf.width = rs.width;
	sf.height = rs.height;
	sf.x = 0;
	sf.y = rs.windowOffset;

	T_GCM_CALL(cellGcmSetSurfaceWindow)(
		gCellGcmCurrentContext,
		&sf,
		CELL_GCM_WINDOW_ORIGIN_BOTTOM,
		CELL_GCM_WINDOW_PIXEL_CENTER_INTEGER
	);

	if (rs.zcull && rs.depthOffset)
	{
		T_GCM_CALL(cellGcmSetZcullEnable)(
			gCellGcmCurrentContext,
			CELL_GCM_TRUE,
			CELL_GCM_TRUE
		);
	}
	else
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
			gcmClearMask |= CELL_GCM_CLEAR_Z/* | CELL_GCM_CLEAR_S*/;
			T_GCM_CALL(cellGcmSetClearDepthStencil)(
				gCellGcmCurrentContext,
				(uint32_t(clearDepth * 0xffffff) << 8) | (rs.clearStencil & 0xff)
			);
		}
		//if (rs.zcull)
		//	T_GCM_CALL(cellGcmSetInvalidateZcull)(gCellGcmCurrentContext);
	}

	if (gcmClearMask)
		T_GCM_CALL(cellGcmSetClearSurface)(gCellGcmCurrentContext, gcmClearMask);

	rs.clearMask = 0;
}

	}
}
