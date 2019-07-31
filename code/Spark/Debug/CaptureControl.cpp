#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Debug/CaptureControl.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.CaptureControl", 0, CaptureControl, ISerializable)

CaptureControl::CaptureControl()
:	m_frameCount(0)
{
}

CaptureControl::CaptureControl(int32_t frameCount)
:	m_frameCount(frameCount)
{
}

void CaptureControl::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"frameCount", m_frameCount);
}

	}
}
