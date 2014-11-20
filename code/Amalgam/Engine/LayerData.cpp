#include "Amalgam/Engine/LayerData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.LayerData", LayerData, ISerializable)

LayerData::LayerData()
:	m_permitTransition(true)
{
}

void LayerData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);

	if (s.getVersion() >= 3)
		s >> Member< bool >(L"permitTransition", m_permitTransition);
}

	}
}
