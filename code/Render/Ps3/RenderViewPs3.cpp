#include <cstring>
#include <cell/gcm.h>
#include <sysutil/sysutil_sysparam.h>
#include "Render/Ps3/RenderViewPs3.h"
#include "Render/Ps3/RenderSystemPs3.h"
#include "Render/Ps3/RenderTargetPs3.h"
#include "Render/Ps3/VertexBufferPs3.h"
#include "Render/Ps3/IndexBufferPs3.h"
#include "Render/Ps3/ShaderPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/Primitives.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

uint8_t c_frameSyncLabelId = 128;
uint8_t c_targetSyncLabelId = 129;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.RenderViewPs3", RenderViewPs3, RenderView)

RenderViewPs3::RenderViewPs3(RenderSystemPs3* renderSystem)
:	m_renderSystem(renderSystem)
,	m_width(0)
,	m_height(0)
,	m_colorPitch(0)
,	m_depthAddr(0)
,	m_depthOffset(0)
,	m_depthPitch(0)
,	m_frameSyncLabelData(0)
,	m_frameCounter(0)
,	m_targetSyncLabelData(0)
,	m_targetCounter(0)
{
	std::memset(m_colorAddr, 0, sizeof(m_colorAddr));
	std::memset(m_colorOffset, 0, sizeof(m_colorOffset));
}

RenderViewPs3::~RenderViewPs3()
{
}

bool RenderViewPs3::create(int resolutionId, int width, int height)
{
	CellVideoOutState videoState;
	CellVideoOutConfiguration videoConfig;
	int32_t ret;

	m_width = width;
	m_height = height;

	m_colorPitch = width * 4;

	memset(&videoConfig, 0, sizeof(CellVideoOutConfiguration));
	videoConfig.resolutionId = resolutionId;
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

	uint32_t colorSize = m_colorPitch * height;
	uint32_t colorBaseAddr = (uint32_t)LocalMemoryAllocator::getInstance().allocAlign(sizeof_array(m_colorAddr) * colorSize, 4096);

	for (size_t i = 0; i < sizeof_array(m_colorAddr); i++)
	{
		m_colorAddr[i] = (void *)(colorBaseAddr + (i * colorSize));

		ret = cellGcmAddressToOffset(m_colorAddr[i], &m_colorOffset[i]);
		if (ret != CELL_OK)
		{
			log::error << L"Create render view failed, unable to create color buffers" << Endl;
			return false;
		}

		ret = cellGcmSetDisplayBuffer(
			i,
			m_colorOffset[i],
			m_colorPitch,
			width,
			height
		);
		if (ret != CELL_OK)
		{
			log::error << L"Create render view failed, unable to set display buffers" << Endl;
			return false;
		}
	}

	m_depthPitch = width * 4;

	uint32_t depthSize = m_depthPitch * height;
	m_depthAddr = LocalMemoryAllocator::getInstance().allocAlign(depthSize, 4096);

	ret = cellGcmAddressToOffset(m_depthAddr, &m_depthOffset);
	if (ret != CELL_OK)
	{
		log::error << L"Create render view failed, unable to create depth buffer" << Endl;
		return false;
	}

	m_frameSyncLabelData = cellGcmGetLabelAddress(c_frameSyncLabelId);
	*m_frameSyncLabelData = m_frameCounter;

	m_targetSyncLabelData = cellGcmGetLabelAddress(c_targetSyncLabelId);
	*m_targetSyncLabelData = m_targetCounter;

	setViewport(Viewport(0, 0, width, height, 0.0f, 1.0f));

	return true;
}

void RenderViewPs3::close()
{
}

void RenderViewPs3::resize(int width, int height)
{
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

bool RenderViewPs3::begin(RenderTarget* renderTarget, RenderMode renderMode)
{
	if (!renderTarget)
	{
		assert (m_renderStateStack.empty());

		uint32_t frameIndex = m_frameCounter % sizeof_array(m_colorOffset);

		RenderState rs =
		{
			m_viewport,
			m_width,
			m_height,
			CELL_GCM_SURFACE_A8R8G8B8,
			m_colorOffset[frameIndex],
			m_colorPitch,
			m_depthOffset,
			m_depthPitch,
			renderMode
		};

		m_renderStateStack.push_back(rs);
	}
	else
	{
		T_ASSERT (!m_renderStateStack.empty());

		RenderTargetPs3* rt = checked_type_cast< RenderTargetPs3* >(renderTarget);

		RenderState rs =
		{
			Viewport(0, 0, rt->getWidth(), rt->getHeight(), 0.0f, 1.0f),
			rt->getWidth(),
			rt->getHeight(),
			rt->getGcmSurfaceColorFormat(),
			rt->getGcmColorTexture().offset,
			rt->getGcmColorTexture().pitch,
			rt->getGcmDepthTexture().offset,
			rt->getGcmDepthTexture().pitch,
			renderMode
		};

		m_renderStateStack.push_back(rs);
	}

	setCurrentRenderState();

	cellGcmSetInvalidateVertexCache(gCellGcmCurrentContext); 
	cellGcmSetInvalidateTextureCache(gCellGcmCurrentContext, CELL_GCM_INVALIDATE_TEXTURE);

	return true;
}

void RenderViewPs3::clear(unsigned int clearMask, unsigned int color, float depth, int stencil)
{
	if (!m_renderStateStack.empty())
	{
		clearMask &= ~CfColor;
		if (!m_renderStateStack.back().depthOffset)
			clearMask &= ~(CfDepth | CfStencil);
	}

	if (!clearMask)
		return;

	uint32_t gcmClear =
		((clearMask & CfColor) ? (CELL_GCM_CLEAR_R | CELL_GCM_CLEAR_G | CELL_GCM_CLEAR_B | CELL_GCM_CLEAR_A) : 0) |
		((clearMask & (CfDepth | CfStencil)) ? (CELL_GCM_CLEAR_Z) : 0);
	
	if (clearMask & CfColor)
		cellGcmSetClearColor(gCellGcmCurrentContext, color);
	if (clearMask & (CfDepth | CfStencil))
		cellGcmSetClearDepthStencil(gCellGcmCurrentContext, (uint32_t(depth * 0xffffff) << 8) | (stencil & 0xff));

	cellGcmSetClearSurface(gCellGcmCurrentContext, gcmClear);
}

void RenderViewPs3::setVertexBuffer(VertexBuffer* vertexBuffer)
{
	m_currentVertexBuffer = checked_type_cast< VertexBufferPs3* >(vertexBuffer);
}

void RenderViewPs3::setIndexBuffer(IndexBuffer* indexBuffer)
{
	m_currentIndexBuffer = checked_type_cast< IndexBufferPs3* >(indexBuffer);
}

void RenderViewPs3::setShader(Shader* shader)
{
	m_currentShader = checked_type_cast< ShaderPs3* >(shader);
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
	T_ASSERT (m_currentShader);

	m_currentVertexBuffer->bind();
	m_currentShader->bind(m_renderStateStack.back().mode);

	const std::vector< Primitives::Set >& sets = primitives.get();
	for (std::vector< Primitives::Set >::const_iterator i = sets.begin(); i != sets.end(); ++i)
	{
		switch (i->type)
		{
		case Primitives::PtPoints:
			count = i->count;
			break;

		case Primitives::PtLineStrip:
			count = i->count + 1;
			break;

		case Primitives::PtLines:
			count = i->count * 2;
			break;

		case Primitives::PtTriangleStrip:
			count = i->count + 2;
			break;

		case Primitives::PtTriangles:
			count = i->count * 3;
			break;
		}

		if (i->indexed)
		{
			T_ASSERT (m_currentIndexBuffer);

			uint8_t type = CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16;
			uint32_t offset = m_currentIndexBuffer->getPs3Offset() + i->offset * 2;

			if (m_currentIndexBuffer->getIndexType() == ItUInt32)
			{
				type = CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32;
				offset += i->offset * 2;
			}
			
			cellGcmSetDrawIndexArray(
				gCellGcmCurrentContext,
				c_mode[i->type],
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
				c_mode[i->type],
				i->offset,
				count
			);
		}
	}
}

void RenderViewPs3::end()
{
	T_ASSERT (!m_renderStateStack.empty());
	
	m_renderStateStack.pop_back();

	if (!m_renderStateStack.empty())
	{
		cellGcmSetWriteBackEndLabel(gCellGcmCurrentContext, c_targetSyncLabelId, m_targetCounter);
		cellGcmFlush(gCellGcmCurrentContext);
		cellGcmSetWaitLabel(gCellGcmCurrentContext, c_targetSyncLabelId, m_targetCounter);

		++m_targetCounter;
		
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

	m_frameCounter++;
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

	if (rs.depthOffset)
	{
		cellGcmSetDepthTestEnable(gCellGcmCurrentContext, CELL_GCM_TRUE);
		cellGcmSetDepthFunc(gCellGcmCurrentContext, CELL_GCM_LEQUAL);
	}
	else
		cellGcmSetDepthTestEnable(gCellGcmCurrentContext, CELL_GCM_FALSE);

}

	}
}
