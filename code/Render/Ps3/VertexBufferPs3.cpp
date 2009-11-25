#include "Render/Ps3/PlatformPs3.h"
#include "Render/Ps3/VertexBufferPs3.h"
#include "Render/Ps3/LocalMemoryAllocator.h"
#include "Render/Ps3/CgType.h"
#include "Render/VertexElement.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferPs3", VertexBufferPs3, VertexBuffer)

VertexBufferPs3::VertexBufferPs3(const std::vector< VertexElement >& vertexElements, void* ptr, uint32_t offset, int bufferSize)
:	VertexBuffer(bufferSize)
,	m_ptr(ptr)
,	m_offset(offset)
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

		if (cellGcmAddressToOffset(static_cast< uint8_t* >(m_ptr) + i->getOffset(), &m_attributeDesc[attr].offset) != CELL_OK)
			log::error << "Unable to get offset to vertex element" << Endl;
	}
}

VertexBufferPs3::~VertexBufferPs3()
{
	destroy();
}

void VertexBufferPs3::destroy()
{
	if (m_ptr)
	{
		LocalMemoryAllocator::getInstance().free(m_ptr);

		m_ptr = 0;
		m_offset = 0;
	}
}

void* VertexBufferPs3::lock()
{
	return m_ptr;
}

void* VertexBufferPs3::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	uint8_t* ptr = static_cast< uint8_t* >(m_ptr);
	ptr += vertexOffset * m_vertexStride;
	return ptr;
}

void VertexBufferPs3::unlock()
{
}

void VertexBufferPs3::bind()
{
	for (int i = 0; i < sizeof_array(m_attributeDesc); ++i)
	{
		if (m_attributeDesc[i].size)
		{
			cellGcmSetVertexDataArray(
				gCellGcmCurrentContext,
				i,
				0,
				m_vertexStride,
				m_attributeDesc[i].size,
				m_attributeDesc[i].type,
				CELL_GCM_LOCATION_LOCAL,
				m_attributeDesc[i].offset
			);
		}
		else
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
	}
}

	}
}
