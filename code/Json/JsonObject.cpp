#include "Json/JsonMember.h"
#include "Json/JsonObject.h"

namespace traktor
{
	namespace json
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.json.JsonObject", JsonObject, JsonNode)

JsonObject::JsonObject()
{
}

void JsonObject::push(JsonMember* member)
{
	m_members.push_back(member);
}

JsonMember* JsonObject::getMember(const std::wstring& name)
{
	for (RefArray< JsonMember >::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

Any JsonObject::getMemberValue(const std::wstring& name)
{
	JsonMember* member = getMember(name);
	return member ? member->getValue() : Any();
}

	}
}
