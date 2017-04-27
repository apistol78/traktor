/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Render/Sw/VertexBufferSw.h"
#include "Render/VertexElement.h"
#include "Core/Memory/Alloc.h"
#include "Core/Math/Half.h"
#include "Core/Math/Float.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

float safeHalfToFloat(half_t half)
{
	float value = halfToFloat(half);
	if (isNan(value) || isInfinite(value))
		return 0.0f;
	else
		return value;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferSw", VertexBufferSw, VertexBuffer)

VertexBufferSw::VertexBufferSw(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBuffer(bufferSize)
,	m_vertexElements(vertexElements)
,	m_vertexStride(getVertexSize(vertexElements))
,	m_vertexCount(bufferSize / m_vertexStride)
,	m_lock(0)
,	m_lockOffset(0)
,	m_lockCount(0)
{
	T_ASSERT (bufferSize % m_vertexStride == 0);
	m_data.reset(new vertex_tuple_t[m_vertexCount * m_vertexElements.size()]);
}

void VertexBufferSw::destroy()
{
	m_data.release();
}

void* VertexBufferSw::lock()
{
	if (m_lock)
		return 0;

	m_lock = (uint8_t*)Alloc::acquireAlign(getBufferSize(), alignOf< Vector4 >(), T_FILE_LINE);
	m_lockOffset = 0;
	m_lockCount = m_vertexCount;

	return m_lock;
}

void* VertexBufferSw::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	if (m_lock)
		return 0;

	m_lock = (uint8_t*)Alloc::acquireAlign(vertexCount * m_vertexStride, alignOf< Vector4 >(), T_FILE_LINE);
	m_lockOffset = vertexOffset;
	m_lockCount = vertexCount;

	return m_lock;
}

void VertexBufferSw::unlock()
{
	if (!m_lock)
		return;

	uint8_t* lockIter = m_lock;
	vertex_tuple_t* ptr = &m_data[m_lockOffset * m_vertexElements.size()];

	for (uint32_t i = 0; i < m_lockCount; ++i)
	{
		for (std::vector< VertexElement >::const_iterator j = m_vertexElements.begin(); j != m_vertexElements.end(); ++j)
		{
			uint8_t* source = lockIter + j->getOffset();

			vertex_tuple_t& out = *ptr++;
			switch (j->getDataType())
			{
			case DtFloat1:
				out[0] = *reinterpret_cast< const float* >(source);
				out[1] = 0.0f;
				out[2] = 0.0f;
				out[3] = (j->getDataUsage() == DuPosition) ? 1.0f : 0.0f;
				break;

			case DtFloat2:
				out[0] = *reinterpret_cast< const float* >(source);
				out[1] = *reinterpret_cast< const float* >(source + sizeof(float));
				out[2] = 0.0f;
				out[3] = (j->getDataUsage() == DuPosition) ? 1.0f : 0.0f;
				break;

			case DtFloat3:
				out[0] = *reinterpret_cast< const float* >(source);
				out[1] = *reinterpret_cast< const float* >(source + sizeof(float));
				out[2] = *reinterpret_cast< const float* >(source + sizeof(float) * 2);
				out[3] = (j->getDataUsage() == DuPosition) ? 1.0f : 0.0f;
				break;

			case DtFloat4:
				out[0] = *reinterpret_cast< const float* >(source);
				out[1] = *reinterpret_cast< const float* >(source + sizeof(float));
				out[2] = *reinterpret_cast< const float* >(source + sizeof(float) * 2);
				out[3] = *reinterpret_cast< const float* >(source + sizeof(float) * 3);
				break;

			case DtByte4:
				out[0] = *reinterpret_cast< const uint8_t* >(source);
				out[1] = *reinterpret_cast< const uint8_t* >(source + 1);
				out[2] = *reinterpret_cast< const uint8_t* >(source + 2);
				out[3] = *reinterpret_cast< const uint8_t* >(source + 3);
				break;

			case DtByte4N:
				out[0] = *reinterpret_cast< const uint8_t* >(source) / 255.0f;
				out[1] = *reinterpret_cast< const uint8_t* >(source + 1) / 255.0f;
				out[2] = *reinterpret_cast< const uint8_t* >(source + 2) / 255.0f;
				out[3] = *reinterpret_cast< const uint8_t* >(source + 3) / 255.0f;
				break;

			case DtShort2:
				out[0] = *reinterpret_cast< const int16_t* >(source);
				out[1] = *reinterpret_cast< const int16_t* >(source + sizeof(short));
				out[2] = 0.0f;
				out[3] = (j->getDataUsage() == DuPosition) ? 1.0f : 0.0f;
				break;

			case DtShort4:
				out[0] = *reinterpret_cast< const int16_t* >(source);
				out[1] = *reinterpret_cast< const int16_t* >(source + sizeof(short));
				out[2] = *reinterpret_cast< const int16_t* >(source + sizeof(short) * 2);
				out[3] = *reinterpret_cast< const int16_t* >(source + sizeof(short) * 3);
				break;

			case DtShort2N:
				out[0] = *reinterpret_cast< const int16_t* >(source) / 32767.0f;
				out[1] = *reinterpret_cast< const int16_t* >(source + sizeof(short)) / 32767.0f;
				out[2] = 0.0f;
				out[3] = (j->getDataUsage() == DuPosition) ? 1.0f : 0.0f;
				break;

			case DtShort4N:
				out[0] = *reinterpret_cast< const int16_t* >(source) / 32767.0f;
				out[1] = *reinterpret_cast< const int16_t* >(source + sizeof(short)) / 32767.0f;
				out[2] = *reinterpret_cast< const int16_t* >(source + sizeof(short) * 2) / 32767.0f;
				out[3] = *reinterpret_cast< const int16_t* >(source + sizeof(short) * 3) / 32767.0f;
				break;

			case DtHalf2:
				out[0] = safeHalfToFloat(*reinterpret_cast< const half_t* >(source));
				out[1] = safeHalfToFloat(*reinterpret_cast< const half_t* >(source + sizeof(half_t)));
				out[2] = 0.0f;
				out[3] = (j->getDataUsage() == DuPosition) ? 1.0f : 0.0f;
				break;

			case DtHalf4:
				out[0] = safeHalfToFloat(*reinterpret_cast< const half_t* >(source));
				out[1] = safeHalfToFloat(*reinterpret_cast< const half_t* >(source + sizeof(half_t)));
				out[2] = safeHalfToFloat(*reinterpret_cast< const half_t* >(source + sizeof(half_t) * 2));
				out[3] = safeHalfToFloat(*reinterpret_cast< const half_t* >(source + sizeof(half_t) * 3));
				break;

			default:
				log::warning << L"Unsupported vertex format" << Endl;
				T_BREAKPOINT;
			}
		}

		lockIter += m_vertexStride;
	}

	Alloc::freeAlign(m_lock);
	m_lock = 0;

	setContentValid(true);
}

	}
}
