#include "Render/Ps3/ClearFpPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"

// Resources
#include "Resources/ClearFp_vertex.h"
#include "Resources/ClearFp_fragment.h"

namespace traktor
{
	namespace render
	{

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
	m_clearColorIndex = cellGcmCgGetParameterResource(m_clearVertexProgram, color) - CG_ATTR0;
}

ClearFpPs3::~ClearFpPs3()
{
}

void ClearFpPs3::clear(const float color[4])
{
	cellGcmSetPerfMonPushMarker(gCellGcmCurrentContext, "Clear FP target");

	cellGcmSetVertexProgram(gCellGcmCurrentContext, m_clearVertexProgram, m_clearVertexProgramUcode);
	cellGcmSetFragmentProgram(gCellGcmCurrentContext, m_clearFragmentProgram, m_clearFragmentProgramUcode->getOffset());
	cellGcmSetVertexDataArray(
		gCellGcmCurrentContext,
		m_clearPositionIndex,
		0, 
		sizeof(float) * 2, 
		2, 
		CELL_GCM_VERTEX_F, 
		CELL_GCM_LOCATION_LOCAL, 
		m_quadBuffer->getOffset()
	);
	cellGcmSetVertexData4f(
		gCellGcmCurrentContext,
		m_clearColorIndex, 
		color
	);

	cellGcmSetDrawArrays(gCellGcmCurrentContext, CELL_GCM_PRIMITIVE_TRIANGLE_STRIP, 0, 4);

	cellGcmSetPerfMonPopMarker(gCellGcmCurrentContext);
}

	}
}
