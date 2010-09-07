#include <cstring>
#include "Core/Log/Log.h"
#include "Render/Ps3/RenderViewPs3.h"
#include "Render/Ps3/RenderSystemPs3.h"
#include "Render/Ps3/RenderTargetSetPs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/VertexBufferPs3.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/ProgramPs3.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_reportZCullStats0 = 100;
const uint32_t c_reportZCullStats1 = 101;
const uint32_t c_patchProgramSize = 1024 * 1024;
const uint32_t c_patchProgramCount = 2;
const uint8_t c_frameSyncLabelId = 128;
const uint8_t c_waitLabelId = 129;

int32_t findResolutionId(int32_t width, int32_t height)
{
	struct { int32_t width; int32_t height; int32_t id; } c_resolutionIds[] =
	{
		{ 1920, 1080, CELL_VIDEO_OUT_RESOLUTION_1080 },
		{ 1280, 720, CELL_VIDEO_OUT_RESOLUTION_720 },
		{ 640, 480, CELL_VIDEO_OUT_RESOLUTION_480 },
		{ 720, 576, CELL_VIDEO_OUT_RESOLUTION_576 },
		{ 1600, 1080, CELL_VIDEO_OUT_RESOLUTION_1600x1080 },
		{ 1440, 1080, CELL_VIDEO_OUT_RESOLUTION_1440x1080 },
		{ 1280, 1080, CELL_VIDEO_OUT_RESOLUTION_1280x1080 },
		{ 960, 1080, CELL_VIDEO_OUT_RESOLUTION_960x1080 }
	};
	for (int32_t i = 0; i < sizeof_array(c_resolutionIds); ++i)
	{
		if (c_resolutionIds[i].width == width && c_resolutionIds[i].height == height)
			return c_resolutionIds[i].id;
	}
	return CELL_VIDEO_OUT_RESOLUTION_UNDEFINED;
}

uint32_t incrementLabel(uint32_t label)
{
	return (++label != 0) ? label : 1;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewPs3", RenderViewPs3, IRenderView)

RenderViewPs3::RenderViewPs3(RenderSystemPs3* renderSystem)
:	m_renderSystem(renderSystem)
,	m_width(0)
,	m_height(0)
,	m_colorObject(0)
,	m_colorPitch(0)
,	m_depthObject(0)
,	m_depthAddr(0)
,	m_frameCounter(1)
,	m_frameSyncLabelData(0)
,	m_renderTargetDirty(false)
,	m_patchProgramObject(0)
{
	std::memset(m_colorAddr, 0, sizeof(m_colorAddr));
	std::memset(m_colorOffset, 0, sizeof(m_colorOffset));
	std::memset(&m_depthTexture, 0, sizeof(m_depthTexture));
	std::memset(m_targetSize, 0, sizeof(m_targetSize));
}

RenderViewPs3::~RenderViewPs3()
{
}

bool RenderViewPs3::create(MemoryHeap* memoryHeap, const RenderViewDefaultDesc& desc)
{
	CellVideoOutState videoState;
	CellVideoOutConfiguration videoConfig;
	int32_t ret;

	// Create FP clear helper.
	m_clearFp.create(memoryHeap);

	m_width = desc.displayMode.width;
	m_height = desc.displayMode.height;

	m_colorPitch = m_width * 4;

	std::memset(&videoConfig, 0, sizeof(CellVideoOutConfiguration));
	videoConfig.resolutionId = findResolutionId(m_width, m_height);
	videoConfig.format = CELL_VIDEO_OUT_BUFFER_COLOR_FORMAT_X8R8G8B8;
	videoConfig.pitch = m_colorPitch;

	ret = cellVideoOutConfigure(CELL_VIDEO_OUT_PRIMARY, &videoConfig, NULL, 0);
	if (ret != CELL_OK)
	{
		log::error << L"Create render view failed, unable to configure video" << Endl;
		return 0;
	}

	ret = cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &videoState);
	if (ret != CELL_OK)
	{
		log::error << L"Create render view failed, unable get video state" << Endl;
		return 0;
	}

	if (desc.waitVBlank)
		cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);
	else
		cellGcmSetFlipMode(CELL_GCM_DISPLAY_HSYNC);

	uint32_t colorSize = m_colorPitch * m_height;
	m_colorObject = memoryHeap->alloc(sizeof_array(m_colorAddr) * colorSize, 4096, true);

	for (size_t i = 0; i < sizeof_array(m_colorAddr); i++)
	{
		m_colorAddr[i] = (uint8_t*)m_colorObject->getPointer() + (i * colorSize);
		m_colorOffset[i] = m_colorObject->getOffset() + (i * colorSize);

		ret = cellGcmSetDisplayBuffer(
			i,
			m_colorOffset[i],
			m_colorPitch,
			m_width,
			m_height
		);
		if (ret != CELL_OK)
		{
			log::error << L"Create render view failed, unable to set display buffers" << Endl;
			return false;
		}
	}

	// Allocate depth buffer.
	{
		int surfaceWidth = m_width;
		int surfaceHeight = m_height;

		m_depthTexture.format = CELL_GCM_TEXTURE_DEPTH24_D8 | CELL_GCM_TEXTURE_LN;
		m_depthTexture.mipmap = 1;
		m_depthTexture.dimension = CELL_GCM_TEXTURE_DIMENSION_2;
		m_depthTexture.cubemap = 0;
		m_depthTexture.remap = 0;
		m_depthTexture.width = surfaceWidth;
		m_depthTexture.height = surfaceHeight;
		m_depthTexture.depth = 1;
		m_depthTexture.location = CELL_GCM_LOCATION_LOCAL;
		m_depthTexture.pitch = surfaceWidth * 4;
		m_depthTexture.offset = 0;

		uint32_t depthSize = m_depthTexture.pitch * m_depthTexture.height;
		m_depthObject = memoryHeap->alloc(depthSize, 4096, true);

		m_depthAddr = m_depthObject->getPointer();
		m_depthTexture.offset = m_depthObject->getOffset();

		// Setup Z-cull binding.
		cellGcmBindZcull(
			0,
			m_depthTexture.offset,
			surfaceWidth,
			surfaceHeight,
			0,
			CELL_GCM_ZCULL_Z24S8,
			CELL_GCM_SURFACE_CENTER_1,
			CELL_GCM_ZCULL_LESS,
			CELL_GCM_ZCULL_LONES,
			CELL_GCM_SCULL_SFUNC_ALWAYS,
			0,
			0
		);
		T_GCM_CALL(cellGcmSetZcullStatsEnable)(
			gCellGcmCurrentContext,
			CELL_GCM_TRUE
		);
	}

	// Create frame synchronization labels.
	m_frameSyncLabelData = cellGcmGetLabelAddress(c_frameSyncLabelId);
	*m_frameSyncLabelData = m_frameCounter;

	volatile uint32_t* waitLabelData = cellGcmGetLabelAddress(c_waitLabelId);
	*waitLabelData = 0;

	// Allocate area for patched pixel programs.
	m_patchProgramObject = memoryHeap->alloc(
		c_patchProgramSize * c_patchProgramCount,
		64,
		false
	);

	// Set default gamma correction.
	const float c_defaultGammaCorrection = 0.8f;
	cellVideoOutSetGamma(CELL_VIDEO_OUT_PRIMARY, c_defaultGammaCorrection);

	setViewport(Viewport(0, 0, m_width, m_height, 0.0f, 1.0f));
	return true;
}

void RenderViewPs3::close()
{
	cellGcmUnbindZcull(0);

	if (m_patchProgramObject)
	{
		m_patchProgramObject->free();
		m_patchProgramObject = 0;
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

bool RenderViewPs3::begin()
{
	m_renderSystem->acquireLock();
	m_renderSystem->compactHeaps();

	uint32_t frameIndex = m_frameCounter % sizeof_array(m_colorOffset);

	RenderState rs =
	{
		m_viewport,
		m_width,
		m_height,
		CELL_GCM_SURFACE_A8R8G8B8,
		m_colorOffset[frameIndex],
		m_colorPitch,
		m_depthTexture.offset,
		m_depthTexture.pitch,
		0,
		true
	};

	T_ASSERT (m_renderTargetStack.empty());
	m_renderTargetStack.push_back(rs);
	m_renderTargetDirty = true;

	// Reset patched program pool.
	uint8_t* patchArea = static_cast< uint8_t* >(m_patchProgramObject->getPointer()) + (m_frameCounter % c_patchProgramCount) * c_patchProgramSize;
	m_patchProgramPool = PoolAllocator(patchArea, c_patchProgramSize);

	/*
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
	*/
	T_GCM_CALL(cellGcmSetZcullLimit)(
		gCellGcmCurrentContext,
		0x100,
		0x100
	);

	// Ensure no program is currently bound; might need to repatch program.
	ProgramPs3::unbind();
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
		rt,
		false
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

	m_currentProgram->bind(m_patchProgramPool, m_stateCache, m_targetSize);
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

#if 0
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
	if (cellGcmSetFlip(gCellGcmCurrentContext, m_frameCounter % sizeof_array(m_colorAddr)) != CELL_OK)
	{
		log::error << L"Failed to present, unable to issue flip" << Endl;
		return;
	}
	T_GCM_CALL(cellGcmSetWaitFlip)(gCellGcmCurrentContext);
	T_GCM_CALL(cellGcmSetWriteBackEndLabel)(gCellGcmCurrentContext, c_frameSyncLabelId, m_frameCounter);
	T_GCM_CALL(cellGcmFlush)(gCellGcmCurrentContext);

	while (*m_frameSyncLabelData + 1 < m_frameCounter)
		sys_timer_usleep(100);

	m_frameCounter = incrementLabel(m_frameCounter);

	m_renderSystem->releaseLock();
}

void RenderViewPs3::pushMarker(const char* const marker)
{
	cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, marker);
}

void RenderViewPs3::popMarker()
{
	cellGcmSetPerfMonPopMarker(gCellGcmCurrentContext);
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
	sf.y = 0;

	T_GCM_CALL(cellGcmSetSurfaceWindow)(
		gCellGcmCurrentContext,
		&sf,
		CELL_GCM_WINDOW_ORIGIN_BOTTOM,
		CELL_GCM_WINDOW_PIXEL_CENTER_INTEGER
	);

	m_stateCache.setViewport(rs.viewport);

	if (rs.colorFormat == CELL_GCM_SURFACE_F_W32Z32Y32X32 || rs.colorFormat == CELL_GCM_SURFACE_F_X32)
		m_stateCache.setInFp32Mode(true);
	else
		m_stateCache.setInFp32Mode(false);

	m_stateCache.reset(/*StateCachePs3::RfRenderState | */StateCachePs3::RfSamplerStates);

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
			gcmClearMask |= CELL_GCM_CLEAR_Z | CELL_GCM_CLEAR_S;
			T_GCM_CALL(cellGcmSetClearDepthStencil)(
				gCellGcmCurrentContext,
				(uint32_t(rs.clearDepth * 0xffffff) << 8) | (rs.clearStencil & 0xff)
			);
		}
		if (rs.zcull)
			T_GCM_CALL(cellGcmSetInvalidateZcull)(gCellGcmCurrentContext);
	}

	if (gcmClearMask)
		T_GCM_CALL(cellGcmSetClearSurface)(gCellGcmCurrentContext, gcmClearMask);

	rs.clearMask = 0;
}

	}
}
