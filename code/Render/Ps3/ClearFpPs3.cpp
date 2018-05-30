/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Ps3/ClearFpPs3.h"
#include "Render/Ps3/MemoryHeap.h"
#include "Render/Ps3/MemoryHeapObject.h"
#include "Render/Ps3/StateCachePs3.h"

// Resources
#include "Resources/ClearVertex.h"
#include "Resources/ClearFragment.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_clearColorRegister = 0;

		}

ClearFpPs3::ClearFpPs3()
:	m_quadBuffer(0)
,	m_quadBufferOffset(0)
,	m_clearVertexProgramUcode(0)
,	m_clearFragmentProgramUcode(0)
,	m_clearPositionIndex(0)
{
}

ClearFpPs3::~ClearFpPs3()
{
	if (m_clearFragmentProgramUcode)
	{
		m_clearFragmentProgramUcode->free();
		m_clearFragmentProgramUcode = 0;
	}
	if (m_quadBuffer)
	{
		m_quadBuffer->free();
		m_quadBuffer = 0;
	}
}

bool ClearFpPs3::create(MemoryHeap* memoryHeap)
{
	uint32_t ucodeSize;
	void* ucode;

	m_quadBuffer = memoryHeap->alloc(sizeof(float) * 4 * 2, 16, false);
	if (!m_quadBuffer)
		return false;

	float* ptr = (float*)m_quadBuffer->getPointer();

	ptr[0] = -1.0f; ptr[1] = -1.0f;
	ptr[2] =  1.0f; ptr[3] = -1.0f;
	ptr[4] = -1.0f; ptr[5] =  1.0f;
	ptr[6] =  1.0f; ptr[7] =  1.0f;

	m_clearVertexProgram = (CGprogram)(void*)&c_ResourceClearVertex;
	m_clearFragmentProgram = (CGprogram)(void*)&c_ResourceClearFragment;

	cellGcmCgInitProgram(m_clearVertexProgram);
	cellGcmCgInitProgram(m_clearFragmentProgram);

	cellGcmCgGetUCode(m_clearFragmentProgram, &ucode, &ucodeSize);
	m_clearFragmentProgramUcode = memoryHeap->alloc(ucodeSize, 64, false);
	std::memcpy(m_clearFragmentProgramUcode->getPointer(), ucode, ucodeSize); 

	cellGcmCgGetUCode(m_clearVertexProgram, &ucode, &ucodeSize);
	m_clearVertexProgramUcode = ucode;

	CGparameter position = cellGcmCgGetNamedParameter(m_clearVertexProgram, "position");
	CGparameter color = cellGcmCgGetNamedParameter(m_clearVertexProgram, "color");

	m_clearPositionIndex = cellGcmCgGetParameterResource(m_clearVertexProgram, position) - CG_ATTR0;

	return true;
}

void ClearFpPs3::clear(StateCachePs3& stateCache, const float color[4])
{
	cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, "Clear FP target");

	//stateCache.reset(StateCachePs3::RfRenderState);
	stateCache.setProgram(
		m_clearVertexProgram, m_clearVertexProgramUcode,
		m_clearFragmentProgram, m_clearFragmentProgramUcode->getOffset(),
		false,
		false
	);

	float vc[4] __attribute__((aligned(16))) = { color[0], color[1], color[2], color[3] };
	stateCache.setVertexShaderConstant(c_clearColorRegister, 1, vc);

	T_GCM_CALL(cellGcmSetVertexDataArray)(
		gCellGcmCurrentContext,
		m_clearPositionIndex,
		0, 
		sizeof(float) * 2, 
		2, 
		CELL_GCM_VERTEX_F, 
		CELL_GCM_LOCATION_LOCAL, 
		m_quadBuffer->getOffset()
	);

	T_GCM_CALL(cellGcmSetDrawArrays)(gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, 0, 4);

	cellGcmSetPerfMonPopMarker(gCellGcmCurrentContext);
}

	}
}
