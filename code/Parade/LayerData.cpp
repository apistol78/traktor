#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Parade/LayerData.h"
#include "Resource/Member.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.LayerData", LayerData, ISerializable)

bool LayerData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
	s >> resource::Member< script::IScriptContext >(L"script", m_script);
	return true;
}

	}
}
