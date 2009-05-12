#include "Render/Sw/VertexBufferSw.h"
#include "Render/VertexElement.h"
#include "Core/Math/Half.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.VertexBufferSw", VertexBufferSw, VertexBuffer)

VertexBufferSw::VertexBufferSw(const std::vector< VertexElement >& vertexElements, uint32_t bufferSize)
:	VertexBuffer(bufferSize)
,	m_vertexElements(vertexElements)
,	m_vertexStride(getVertexSize(vertexElements))
,	m_data(vertexElements.size() * bufferSize / m_vertexStride)
,	m_lock(0)
,	m_lockOffset(0)
,	m_lockCount(0)
{
	T_ASSERT (bufferSize % m_vertexStride == 0);
}

void VertexBufferSw::destroy()
{
}

void* VertexBufferSw::lock()
{
	if (m_lock)
		return 0;

	m_lock = new uint8_t [getBufferSize()];
	m_lockOffset = 0;
	m_lockCount = getBufferSize() / m_vertexStride;

	return m_lock;
}

void* VertexBufferSw::lock(uint32_t vertexOffset, uint32_t vertexCount)
{
	if (m_lock)
		return 0;

	m_lock = new uint8_t [vertexCount * m_vertexStride];
	m_lockOffset = vertexOffset;
	m_lockCount = vertexCount;

	return m_lock;
}

void VertexBufferSw::unlock()
{
	if (!m_lock)
		return;

	uint8_t* lockIter = m_lock;

	for (uint32_t offset = m_lockOffset; offset < m_lockOffset + m_lockCount; )
	{
		for (std::vector< VertexElement >::const_iterator j = m_vertexElements.begin(); j != m_vertexElements.end(); ++j, ++offset)
		{
			uint8_t* source = lockIter + j->getOffset();

			Vector4& out = m_data[offset];
			switch (j->getDataType())
			{
			case DtFloat1:
				out.set(
					*reinterpret_cast< const float* >(source),
					0.0f,
					0.0f,
					(j->getDataUsage() == DuPosition) ? 1.0f : 0.0f
				);
				break;

			case DtFloat2:
				out.set(
					*reinterpret_cast< const float* >(source),
					*reinterpret_cast< const float* >(source + sizeof(float)),
					0.0f,
					(j->getDataUsage() == DuPosition) ? 1.0f : 0.0f
				);
				break;

			case DtFloat3:
				out.set(
					*reinterpret_cast< const float* >(source),
					*reinterpret_cast< const float* >(source + sizeof(float)),
					*reinterpret_cast< const float* >(source + sizeof(float) * 2),
					(j->getDataUsage() == DuPosition) ? 1.0f : 0.0f
				);
				break;

			case DtFloat4:
				out.set(
					*reinterpret_cast< const float* >(source),
					*reinterpret_cast< const float* >(source + sizeof(float)),
					*reinterpret_cast< const float* >(source + sizeof(float) * 2),
					*reinterpret_cast< const float* >(source + sizeof(float) * 3)
				);
				break;

			case DtByte4:
				out.set(
					*reinterpret_cast< unsigned char* >(source),
					*reinterpret_cast< unsigned char* >(source + 1),
					*reinterpret_cast< unsigned char* >(source + 2),
					*reinterpret_cast< unsigned char* >(source + 3)
				);
				break;

			case DtByte4N:
				out.set(
					*reinterpret_cast< unsigned char* >(source) / 255.0f,
					*reinterpret_cast< unsigned char* >(source + 1) / 255.0f,
					*reinterpret_cast< unsigned char* >(source + 2) / 255.0f,
					*reinterpret_cast< unsigned char* >(source + 3) / 255.0f
				);
				break;

			case DtShort2:
				out.set(
					*reinterpret_cast< short* >(source),
					*reinterpret_cast< short* >(source + sizeof(short)),
					0.0f,
					0.0f
				);
				break;

			case DtShort4:
				out.set(
					*reinterpret_cast< short* >(source),
					*reinterpret_cast< short* >(source + sizeof(short)),
					*reinterpret_cast< short* >(source + sizeof(short) * 2),
					*reinterpret_cast< short* >(source + sizeof(short) * 3)
				);
				break;

			case DtShort2N:
				out.set(
					*reinterpret_cast< short* >(source) / 32767.0f,
					*reinterpret_cast< short* >(source + sizeof(short)) / 32767.0f,
					0.0f,
					0.0f
				);
				break;

			case DtShort4N:
				out.set(
					*reinterpret_cast< short* >(source) / 32767.0f,
					*reinterpret_cast< short* >(source + sizeof(short)) / 32767.0f,
					*reinterpret_cast< short* >(source + sizeof(short) * 2) / 32767.0f,
					*reinterpret_cast< short* >(source + sizeof(short) * 3) / 32767.0f
				);
				break;

			case DtHalf2:
				out.set(
					halfToFloat(*reinterpret_cast< half_t* >(source)),
					halfToFloat(*reinterpret_cast< half_t* >(source + sizeof(half_t))),
					0.0f,
					0.0f
				);
				break;

			case DtHalf4:
				out.set(
					halfToFloat(*reinterpret_cast< half_t* >(source)),
					halfToFloat(*reinterpret_cast< half_t* >(source + sizeof(half_t))),
					halfToFloat(*reinterpret_cast< half_t* >(source + sizeof(half_t) * 2)),
					halfToFloat(*reinterpret_cast< half_t* >(source + sizeof(half_t) * 3))
				);
				break;

			default:
				log::warning << L"Unsupported vertex format" << Endl;
				T_BREAKPOINT;
			}
		}

		lockIter += m_vertexStride;
	}

	delete[] m_lock;
	m_lock = 0;
}

	}
}
