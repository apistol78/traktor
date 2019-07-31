#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spark/Debug/MovieDebugInfo.h"

namespace traktor
{
	namespace spark
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spark.MovieDebugInfo", 0, MovieDebugInfo, ISerializable)

MovieDebugInfo::MovieDebugInfo()
{
}

MovieDebugInfo::MovieDebugInfo(const std::wstring& name)
:	m_name(name)
{
}

void MovieDebugInfo::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
