#include "Render/Ps3/ClearFpPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"

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

	m_quadBuffer = (float*)LocalMemoryAllocator::getInstance().allocAlign(sizeof(float) * 4 * 2, 128);
	cellGcmAddressToOffset((void*)m_quadBuffer, &m_quadBufferOffset);

	m_quadBuffer[0] = -1.0f; m_quadBuffer[1] = -1.0f;
	m_quadBuffer[2] =  1.0f; m_quadBuffer[3] = -1.0f;
	m_quadBuffer[4] = -1.0f; m_quadBuffer[5] =  1.0f;
	m_quadBuffer[6] =  1.0f; m_quadBuffer[7] =  1.0f;

	m_clearVertexProgram = (CGprogram)c_ResourceClearFp_vertex;
	m_clearFragmentProgram = (CGprogram)c_ResourceClearFp_fragment;

	cellGcmCgInitProgram(m_clearVertexProgram);
	cellGcmCgInitProgram(m_clearFragmentProgram);

	cellGcmCgGetUCode(m_clearFragmentProgram, &ucode, &ucodeSize);
	m_clearFragmentProgramUcode = LocalMemoryAllocator::getInstance().allocAlign(ucodeSize, 64);
	std::memcpy(m_clearFragmentProgramUcode, ucode, ucodeSize); 
	cellGcmAddressToOffset(m_clearFragmentProgramUcode, &m_clearFragmentProgramOffset);

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
	cellGcmSetFragmentProgram(gCellGcmCurrentContext, m_clearFragmentProgram, m_clearFragmentProgramOffset);
	cellGcmSetVertexDataArray(
		gCellGcmCurrentContext,
		m_clearPositionIndex,
		0, 
		sizeof(float) * 2, 
		2, 
		CELL_GCM_VERTEX_F, 
		CELL_GCM_LOCATION_LOCAL, 
		m_quadBufferOffset
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
