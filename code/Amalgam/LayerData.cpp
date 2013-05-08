#include "Amalgam/LayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.LayerData", LayerData, ISerializable)

void LayerData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

	}
}
