#include <cstring>
#include "Core/Log/Log.h"
#include "Render/Ps3/RenderViewPs3.h"
#include "Render/Ps3/RenderSystemPs3.h"
#include "Render/Ps3/RenderTargetSetPs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/VertexBufferPs3.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/ProgramPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

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
{
	std::memset(m_colorAddr, 0, sizeof(m_colorAddr));
	std::memset(m_colorOffset, 0, sizeof(m_colorOffset));
	std::memset(&m_depthTexture, 0, sizeof(m_depthTexture));
}

RenderViewPs3::~RenderViewPs3()
{
}

bool RenderViewPs3::create(const DisplayMode* displayMode, const RenderViewCreateDesc& desc)
{
	CellVideoOutState videoState;
	CellVideoOutConfiguration videoConfig;
	int32_t ret;

	m_width = displayMode->getWidth();
	m_height = displayMode->getHeight();

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

	cellGcmSetFlipMode(CELL_GCM_DISPLAY_VSYNC);

	uint32_t colorSize = m_colorPitch * m_height;
	m_colorObject = LocalMemoryManager::getInstance().alloc(sizeof_array(m_colorAddr) * colorSize, 4096, true);

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
		m_depthObject = LocalMemoryManager::getInstance().alloc(depthSize, 4096, true);

		m_depthAddr = m_depthObject->getPointer();
		m_depthTexture.offset = m_depthObject->getOffset();
	}

	m_frameSyncLabelData = cellGcmGetLabelAddress(c_frameSyncLabelId);
	*m_frameSyncLabelData = m_frameCounter;

	volatile uint32_t* waitLabelData = cellGcmGetLabelAddress(c_waitLabelId);
	*waitLabelData = 0;

	setViewport(Viewport(0, 0, m_width, m_height, 0.0f, 1.0f));
	return true;
}

void RenderViewPs3::close()
{
	if (m_depthObject)
	{
		LocalMemoryManager::getInstance().free(m_depthObject);
		m_depthObject = 0;
	}
	if (m_colorObject)
	{
		LocalMemoryManager::getInstance().free(m_colorObject);
		m_colorObject = 0;
	}
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

void RenderViewPs3::setViewport(const Viewport& viewport)
{
	if (m_renderStateStack.empty())
		m_viewport = viewport;
	else
	{
		RenderState& rs = m_renderStateStack.back();
		rs.viewport = viewport;

		float scale[4];
		float offset[4];

		scale[0] = rs.viewport.width * 0.5f;
		scale[1] = rs.viewport.height * -0.5f;
		scale[2] = (rs.viewport.farZ - rs.viewport.nearZ) * 0.5f;
		scale[3] = 0.0f;

		offset[0] = rs.viewport.left + scale[0];
		offset[1] = rs.viewport.height - rs.viewport.top + scale[1];
		offset[2] = (rs.viewport.farZ + rs.viewport.nearZ) * 0.5f;
		offset[3] = 0.0f;

		cellGcmSetViewport(
			gCellGcmCurrentContext,
			rs.viewport.left,
			rs.viewport.top,
			rs.viewport.width,
			rs.viewport.height,
			rs.viewport.nearZ,
			rs.viewport.farZ,
			scale,
			offset
		);

		cellGcmSetScissor(
			gCellGcmCurrentContext,
			rs.viewport.left,
			rs.viewport.top,
			rs.viewport.width,
			rs.viewport.height
		);
	}
}

Viewport RenderViewPs3::getViewport()
{
	return m_viewport;
}

bool RenderViewPs3::getNativeAspectRatio(float& outAspectRatio) const
{
	return false;
}

bool RenderViewPs3::begin()
{
	LocalMemoryManager::getInstance().compact();

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
		0
	};

	T_ASSERT (m_renderStateStack.empty());
	m_renderStateStack.push_back(rs);

	setCurrentRenderState();

	cellGcmSetInvalidateVertexCache(gCellGcmCurrentContext); 
	cellGcmSetInvalidateTextureCache(gCellGcmCurrentContext, CELL_GCM_INVALIDATE_TEXTURE);

	m_stateCache.resetTextures();

	return true;
}

bool RenderViewPs3::begin(RenderTargetSet* renderTargetSet, int renderTarget, bool keepDepthStencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderTargetSetPs3* rts = checked_type_cast< RenderTargetSetPs3* >(renderTargetSet);
	RenderTargetPs3* rt = rts->getRenderTarget(renderTarget);
	T_ASSERT (rt);

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
		rt
	};

	if (keepDepthStencil)
	{
		rs.depthOffset = m_renderStateStack.back().depthOffset;
		rs.depthPitch = m_renderStateStack.back().depthPitch;
	}

	m_renderStateStack.push_back(rs);

	setCurrentRenderState();

	cellGcmSetInvalidateVertexCache(gCellGcmCurrentContext); 
	cellGcmSetInvalidateTextureCache(gCellGcmCurrentContext, CELL_GCM_INVALIDATE_TEXTURE);

	rt->beginRender();

	return true;
}

void RenderViewPs3::clear(uint32_t clearMask, const float color[4], float depth, int32_t stencil)
{
	T_ASSERT (!m_renderStateStack.empty());

	const RenderState& rs = m_renderStateStack.back();
	uint32_t gcmClearMask = 0;

	if (clearMask & CfColor)
	{
		if (
			rs.colorFormat == CELL_GCM_SURFACE_A8R8G8B8 ||
			rs.colorFormat == CELL_GCM_SURFACE_B8
		)
		{
			gcmClearMask |= CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A;

			uint32_t clearColor =
				(uint32_t(color[0] * 255.0f) << 16) |
				(uint32_t(color[1] * 255.0f) << 8) |
				(uint32_t(color[2] * 255.0f) << 0) |
				(uint32_t(color[3] * 255.0f) << 24);

			cellGcmSetClearColor(gCellGcmCurrentContext, clearColor);
		}
		else
		{
			m_stateCache.reset(false);
			m_clearFp.clear(m_stateCache, color);
		}
	}

	if (clearMask & (CfDepth | CfStencil))
	{
		if (rs.depthOffset)
		{
			gcmClearMask |= CELL_GCM_CLEAR_Z;

			cellGcmSetClearDepthStencil(
				gCellGcmCurrentContext,
				(uint32_t(depth * 0xffffff) << 8) | (stencil & 0xff)
			);
		}
	}

	if (gcmClearMask)
		cellGcmSetClearSurface(gCellGcmCurrentContext, gcmClearMask);
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

	uint32_t count = 0;

	T_ASSERT (m_currentVertexBuffer);
	T_ASSERT (m_currentProgram);

	m_currentVertexBuffer->bind();
	m_currentProgram->bind(m_stateCache);

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
		
		cellGcmSetDrawIndexArray(
			gCellGcmCurrentContext,
			c_mode[primitives.type],
			count,
			type,
			CELL_GCM_LOCATION_LOCAL,
			offset
		);
	}
	else
	{
		cellGcmSetDrawArrays(
			gCellGcmCurrentContext,
			c_mode[primitives.type],
			primitives.offset,
			count
		);
	}
}

void RenderViewPs3::end()
{
	T_ASSERT (!m_renderStateStack.empty());

	RenderState& rs = m_renderStateStack.back();
	RenderTargetPs3* rt = rs.renderTarget;
	
	m_renderStateStack.pop_back();

	if (!m_renderStateStack.empty())
	{
		T_ASSERT (rt);
		rt->finishRender();
		setCurrentRenderState();
	}
}

void RenderViewPs3::present()
{
	if (cellGcmSetFlip(gCellGcmCurrentContext, m_frameCounter % sizeof_array(m_colorAddr)) != CELL_OK)
	{
		log::error << L"Failed to present, unable to issue flip" << Endl;
		return;
	}
	cellGcmSetWaitFlip(gCellGcmCurrentContext);

	cellGcmSetWriteBackEndLabel(gCellGcmCurrentContext, c_frameSyncLabelId, m_frameCounter);
	cellGcmFlush(gCellGcmCurrentContext);

	while (*m_frameSyncLabelData + 2 < m_frameCounter)
		sys_timer_usleep(100);

	m_frameCounter = incrementLabel(m_frameCounter);
}

void RenderViewPs3::setMSAAEnable(bool msaaEnable)
{
}

void RenderViewPs3::setCurrentRenderState()
{
	const RenderState& rs = m_renderStateStack.back();

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

	cellGcmSetSurface(gCellGcmCurrentContext, &sf);

	float scale[4];
	float offset[4];

	scale[0] = rs.viewport.width * 0.5f;
	scale[1] = rs.viewport.height * -0.5f;
	scale[2] = (rs.viewport.farZ - rs.viewport.nearZ) * 0.5f;
	scale[3] = 0.0f;

	offset[0] = rs.viewport.left + scale[0];
	offset[1] = rs.viewport.height - rs.viewport.top + scale[1];
	offset[2] = (rs.viewport.farZ + rs.viewport.nearZ) * 0.5f;
	offset[3] = 0.0f;

	cellGcmSetViewport(
		gCellGcmCurrentContext,
		rs.viewport.left,
		rs.viewport.top,
		rs.viewport.width,
		rs.viewport.height,
		rs.viewport.nearZ,
		rs.viewport.farZ,
		scale,
		offset
	);

	cellGcmSetScissor(
		gCellGcmCurrentContext,
		rs.viewport.left,
		rs.viewport.top,
		rs.viewport.width,
		rs.viewport.height
	);

	cellGcmSetColorMask(gCellGcmCurrentContext, CELL_GCM_COLOR_MASK_B | CELL_GCM_COLOR_MASK_G | CELL_GCM_COLOR_MASK_R | CELL_GCM_COLOR_MASK_A);
	cellGcmSetColorMaskMrt(gCellGcmCurrentContext, 0);

	if (rs.colorFormat == CELL_GCM_SURFACE_F_W32Z32Y32X32 || rs.colorFormat == CELL_GCM_SURFACE_F_X32)
		m_stateCache.setInFp32Mode(true);
	else
		m_stateCache.setInFp32Mode(false);

	m_stateCache.reset(false);
}

	}
}
