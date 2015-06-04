#include "Amalgam/CapturedScreenShot.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.CapturedScreenShot", 0, CapturedScreenShot, ISerializable)

CapturedScreenShot::CapturedScreenShot()
:	m_width(0)
,	m_height(0)
{
}

CapturedScreenShot::CapturedScreenShot(int32_t width, int32_t height)
:	m_width(width)
,	m_height(height)
{
	m_data.resize(width * height, 0);
}

void CapturedScreenShot::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"width", m_width);
	s >> Member< int32_t >(L"height", m_height);

	if (s.getDirection() == ISerializer::SdRead)
		m_data.resize(m_width * m_height);

	uint32_t size = uint32_t(m_data.size() * sizeof(uint32_t));
	s >> Member< void* >(L"data", m_data.ptr(), size);
}

	}
}
