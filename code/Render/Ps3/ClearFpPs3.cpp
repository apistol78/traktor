#include "Render/Ps3/ClearFpPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"
#include "Render/Ps3/StateCachePs3.h"

// Resources
#include "Resources/ClearFp_vertex.h"
#include "Resources/ClearFp_fragment.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const uint32_t c_clearColorRegister = 0;

		}

ClearFpPs3::ClearFpPs3()
{
	uint32_t ucodeSize;
	void* ucode;

	m_quadBuffer = LocalMemoryManager::getInstance().alloc(sizeof(float) * 4 * 2, 128, false);

	float* ptr = (float*)m_quadBuffer->getPointer();

	ptr[0] = -1.0f; ptr[1] = -1.0f;
	ptr[2] =  1.0f; ptr[3] = -1.0f;
	ptr[4] = -1.0f; ptr[5] =  1.0f;
	ptr[6] =  1.0f; ptr[7] =  1.0f;

	m_clearVertexProgram = (CGprogram)c_ResourceClearFp_vertex;
	m_clearFragmentProgram = (CGprogram)c_ResourceClearFp_fragment;

	cellGcmCgInitProgram(m_clearVertexProgram);
	cellGcmCgInitProgram(m_clearFragmentProgram);

	cellGcmCgGetUCode(m_clearFragmentProgram, &ucode, &ucodeSize);
	m_clearFragmentProgramUcode = LocalMemoryManager::getInstance().alloc(ucodeSize, 64, false);
	std::memcpy(m_clearFragmentProgramUcode->getPointer(), ucode, ucodeSize); 

	cellGcmCgGetUCode(m_clearVertexProgram, &ucode, &ucodeSize);
	m_clearVertexProgramUcode = ucode;

	CGparameter position = cellGcmCgGetNamedParameter(m_clearVertexProgram, "position");
	CGparameter color = cellGcmCgGetNamedParameter(m_clearVertexProgram, "color");

	m_clearPositionIndex = cellGcmCgGetParameterResource(m_clearVertexProgram, position) - CG_ATTR0;
}

ClearFpPs3::~ClearFpPs3()
{
	if (m_clearFragmentProgramUcode)
	{
		LocalMemoryManager::getInstance().free(m_clearFragmentProgramUcode);
		m_clearFragmentProgramUcode = 0;
	}
	if (m_quadBuffer)
	{
		LocalMemoryManager::getInstance().free(m_quadBuffer);
		m_quadBuffer = 0;
	}
}

void ClearFpPs3::clear(StateCachePs3& stateCache, const float color[4])
{
	cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, "Clear FP target");

	stateCache.reset(false);
	stateCache.setProgram(
		m_clearVertexProgram, m_clearVertexProgramUcode,
		m_clearFragmentProgram, m_clearFragmentProgramUcode->getOffset(),
		false
	);
	stateCache.setVertexShaderConstant(c_clearColorRegister, 1, color);

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
