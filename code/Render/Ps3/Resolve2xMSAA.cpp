#include "Core/Log/Log.h"
#include "Core/Misc/TString.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/Resolve2xMSAA.h"
#include "Render/Ps3/StateCachePs3.h"

#define USE_DEBUG_DRAW 0

extern "C" unsigned long _binary_Resolve_vpo_start;
extern "C" unsigned long _binary_Resolve2xAccuview_fpo_start;

namespace traktor
{
	namespace render
	{
		namespace
		{

#pragma pack(1)
struct Vertex
{
	float position[2];
	float tex0[2];
};
#pragma pack()

int32_t getParameterIndex(CGprogram program, const char* name, int32_t base)
{
	CGparameter parameter = cellGcmCgGetNamedParameter(program, name);
	if (parameter == 0)
	{
		log::debug << L"Parameter \"" << mbstows(name) << L"\" not used in resolve shader" << Endl;
		return -1;
	}

	return cellGcmCgGetParameterResource(program, parameter) - base;
}

		}

Resolve2xMSAA::Resolve2xMSAA()
:	m_vertexProgramUcode(0)
,	m_fragmentProgramUcode(0)
,	m_positionIndex(-1)
,	m_tex0Index(-1)
,	m_texture0Index(-1)
,	m_texture1Index(-1)
,	m_lastWidth(0)
,	m_lastHeight(0)
,	m_quadBufferCounter(0)
{
	std::memset(m_quadBuffer, 0, sizeof(m_quadBuffer));
}

Resolve2xMSAA::~Resolve2xMSAA()
{
}

bool Resolve2xMSAA::create(MemoryHeap* memoryHeap)
{
	uint32_t ucodeSize;
	void* ucode;

	for (int32_t i = 0; i < sizeof_array(m_quadBuffer); ++i)
	{
		m_quadBuffer[i] = memoryHeap->alloc(sizeof(Vertex) * 4, 16, false);
		if (!m_quadBuffer[i])
			return false;
	}

	m_vertexProgram = (CGprogram)(void*)&_binary_Resolve_vpo_start;
	m_fragmentProgram = (CGprogram)(void*)&_binary_Resolve2xAccuview_fpo_start;

	cellGcmCgInitProgram(m_vertexProgram);
	cellGcmCgInitProgram(m_fragmentProgram);

	cellGcmCgGetUCode(m_vertexProgram, &ucode, &ucodeSize);
	m_vertexProgramUcode = ucode;

	cellGcmCgGetUCode(m_fragmentProgram, &ucode, &ucodeSize);
	m_fragmentProgramUcode = memoryHeap->alloc(ucodeSize, 64, false);
	std::memcpy(m_fragmentProgramUcode->getPointer(), ucode, ucodeSize); 

	m_positionIndex = getParameterIndex(m_vertexProgram, "position", CG_ATTR0);
	m_tex0Index = getParameterIndex(m_vertexProgram, "tex0", CG_ATTR0);
	m_texture0Index = getParameterIndex(m_fragmentProgram, "texture0", CG_TEXUNIT0);
	m_texture1Index = getParameterIndex(m_fragmentProgram, "texture1", CG_TEXUNIT0);

	return true;
}

void Resolve2xMSAA::resolve(StateCachePs3& stateCache, const CellGcmTexture* resolvedTexture, const CellGcmTexture* sourceTexture)
{
	if (m_lastWidth != sourceTexture->width || m_lastHeight != sourceTexture->height)
	{
		m_quadBufferCounter = (m_quadBufferCounter + 1) % sizeof_array(m_quadBuffer);

		float du = -0.5f / float(sourceTexture->width);
		float dv = -0.5f / float(sourceTexture->width);

		Vertex* v = (Vertex*)m_quadBuffer[m_quadBufferCounter]->getPointer();
		v->position[0] = -1.0f; v->position[1] =  1.0f; v->tex0[0] = 0.0f + du; v->tex0[1] = 0.0f + dv; ++v;
		v->position[0] = -1.0f; v->position[1] = -1.0f; v->tex0[0] = 0.0f + du; v->tex0[1] = 1.0f + dv; ++v;
		v->position[0] =  1.0f; v->position[1] = -1.0f; v->tex0[0] = 1.0f + du; v->tex0[1] = 1.0f + dv; ++v;
		v->position[0] =  1.0f; v->position[1] =  1.0f; v->tex0[0] = 1.0f + du; v->tex0[1] = 0.0f + dv; ++v;
	}

	CellGcmSurface sf;
	sf.type = CELL_GCM_SURFACE_PITCH;
	sf.antialias = CELL_GCM_SURFACE_CENTER_1;
	sf.colorFormat = CELL_GCM_SURFACE_A8R8G8B8;
	sf.colorTarget= CELL_GCM_SURFACE_TARGET_0;
	sf.colorLocation[0]	= CELL_GCM_LOCATION_LOCAL;
	sf.colorOffset[0] = resolvedTexture->offset;
	sf.colorPitch[0] = resolvedTexture->pitch;
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
	sf.depthOffset = 0;
	sf.depthPitch = 64;
	sf.width = resolvedTexture->width;
	sf.height = resolvedTexture->height;
	sf.x = 0;
	sf.y = 0;

	T_GCM_CALL(cellGcmSetSurfaceWindow)(
		gCellGcmCurrentContext,
		&sf,
		CELL_GCM_WINDOW_ORIGIN_TOP,
		CELL_GCM_WINDOW_PIXEL_CENTER_HALF
	);

	T_GCM_CALL(cellGcmSetAntiAliasingControl)(
		gCellGcmCurrentContext,
		CELL_GCM_FALSE,
		CELL_GCM_FALSE,
		CELL_GCM_FALSE,
		0xffff
	);

	stateCache.setViewport(Viewport(0, 0, sf.width, sf.height, 0.0f, 1.0f));

	RenderStateGCM rs;
	rs.depthMask = CELL_GCM_FALSE;
	stateCache.setRenderState(rs);

	stateCache.setProgram(
		m_vertexProgram, m_vertexProgramUcode,
		m_fragmentProgram, m_fragmentProgramUcode->getOffset(),
		false,
		false
	);

	SamplerStateGCM ss;
	if (m_texture0Index >= 0)
	{
		stateCache.setSamplerState(m_texture0Index, ss);
		stateCache.setSamplerTexture(m_texture0Index, sourceTexture, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
	}
	if (m_texture1Index >= 0)
	{
		stateCache.setSamplerState(m_texture1Index, ss);
		stateCache.setSamplerTexture(m_texture1Index, sourceTexture, 0, CELL_GCM_TEXTURE_MAX_ANISO_1);
	}

	if (m_positionIndex >= 0)
	{
		stateCache.setVertexDataArray(
			m_positionIndex,
			sizeof(Vertex),
			2,
			CELL_GCM_VERTEX_F,
			m_quadBuffer[m_quadBufferCounter]->getLocation(), 
			m_quadBuffer[m_quadBufferCounter]->getOffset() + offsetof(Vertex, position)
		);
	}
	if (m_tex0Index >= 0)
	{
		stateCache.setVertexDataArray(
			m_tex0Index,
			sizeof(Vertex),
			2,
			CELL_GCM_VERTEX_F,
			m_quadBuffer[m_quadBufferCounter]->getLocation(), 
			m_quadBuffer[m_quadBufferCounter]->getOffset() + offsetof(Vertex, tex0)
		);
	}

	T_GCM_CALL(cellGcmSetDrawArrays)(gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_QUADS, 0, 4);

#if USE_DEBUG_DRAW
	// Synchronize RSX after each draw; help to find RSX crashes.
	static uint32_t s_finishRef = 10000; s_finishRef++;
	cellGcmFinish(gCellGcmCurrentContext, s_finishRef);
#endif
}

	}
}
