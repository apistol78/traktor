#include "Runtime/Engine/LayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.LayerData", LayerData, ISerializable)

void LayerData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> Member< bool >(L"permitTransition", m_permitTransition);
}

	}
}
