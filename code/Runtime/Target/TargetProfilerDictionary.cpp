#include "Runtime/Target/TargetProfilerDictionary.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberSmallMap.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.TargetProfilerDictionary", 0, TargetProfilerDictionary, ISerializable)

TargetProfilerDictionary::TargetProfilerDictionary(const SmallMap< uint16_t, std::wstring >& dictionary)
:	m_dictionary(dictionary)
{
}

void TargetProfilerDictionary::serialize(ISerializer& s)
{
	s >> MemberSmallMap< uint16_t, std::wstring >(L"dictionary", m_dictionary);
}

	}
}
