#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/VertexBufferPs3.h"
#include "Render/Ps3/LocalMemoryManager.h"
#include "Render/Ps3/LocalMemoryObject.h"
#include "Render/Ps3/CgType.h"
#include "Render/VertexElement.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

VertexBufferPs3* VertexBufferPs3::ms_activeVertexBuffer = 0;
bool VertexBufferPs3::ms_attributeEnable[16] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferPs3", VertexBufferPs3, VertexBuffer)

VertexBufferPs3::VertexBufferPs3(const std::vector< VertexElement >& vertexElements, LocalMemoryObject* vbo, int bufferSize)
:	VertexBuffer(bufferSize)
,	m_vbo(vbo)
{
	m_vertexStride = getVertexSize(vertexElements);

	std::memset(m_attributeDesc, 0, sizeof(m_attributeDesc));

	for (std::vector< VertexElement >::const_iterator i = vertexElements.begin(); i != vertexElements.end(); ++i)
	{
		int32_t attr = cg_attr_index(i->getDataUsage(), i->getIndex());
		switch (i->getDataType())
		{
		case DtFloat1:
			m_attributeDesc[attr].size = 1;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtFloat2:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtFloat3:
			m_attributeDesc[attr].size = 3;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtFloat4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_F;
			break;

		case DtByte4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_UB256;
			break;

		case DtByte4N:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_UB;
			break;

		case DtShort2:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S32K;
			break;

		case DtShort4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S32K;
			break;

		case DtShort2N:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S1;

		case DtShort4N:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_S1;
			break;

		case DtHalf2:
			m_attributeDesc[attr].size = 2;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_SF;
			break;

		case DtHalf4:
			m_attributeDesc[attr].size = 4;
			m_attributeDesc[attr].type = CELL_GCM_VERTEX_SF;
			break;
		}

		m_attributeDesc[attr].offset = i->getOffset();
	}
}

VertexBufferPs3::~VertexBufferPs3()
{
	destroy();
}

void VertexBufferPs3::destroy()
{
	if (ms_activeVertexBuffer == this)
	{
		for (int i = 0; i < sizeof_array(m_attributeDesc); ++i)
		{
			cellGcmSetVertexDataArray(
				gCellGcmCurrentContext,
				i,
				0,
				0,
				0,
				CELL_GCM_VERTEX_F,
				CELL_GCM_LOCATION_LOCAL,
				0
			);
		}

		ms_activeVertexBuffer = 0;
	}

	if (m_vbo)
	{
		LocalMemoryManager::getInstance().free(m_vbo);
		m_vbo = 0;
	}
}

void* VertexBufferPs3::lock()
{
	return m_vbo->getPointer();
}

void* VertexBufferPs3::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	uint8_t* ptr = static_cast< uint8_t* >(m_vbo->getPointer());
	ptr += vertexOffset * m_vertexStride;
	return ptr;
}

void VertexBufferPs3::unlock()
{
}

void VertexBufferPs3::bind(const std::vector< uint8_t >& signature)
{
	//if (ms_activeVertexBuffer == this)
	//	return;

	for (int i = 0; i < sizeof_array(m_attributeDesc); ++i)
	{
		if (m_attributeDesc[i].size && signature[i])
		{
			T_GCM_CALL(cellGcmSetVertexDataArray)(
				gCellGcmCurrentContext,
				i,
				0,
				m_vertexStride,
				m_attributeDesc[i].size,
				m_attributeDesc[i].type,
				CELL_GCM_LOCATION_LOCAL,
				m_vbo->getOffset() + m_attributeDesc[i].offset
			);
			ms_attributeEnable[i] = true;
		}
		else if (ms_attributeEnable[i])
		{
			T_GCM_CALL(cellGcmSetVertexDataArray)(
				gCellGcmCurrentContext,
				i,
				0,
				0,
				0,
				CELL_GCM_VERTEX_F,
				CELL_GCM_LOCATION_LOCAL,
				0
			);
			ms_attributeEnable[i] = false;
		}
	}

	ms_activeVertexBuffer = this;
}

	}
}
