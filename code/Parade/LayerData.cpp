#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Parade/LayerData.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.LayerData", LayerData, ISerializable)

bool LayerData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	return true;
}

	}
}
