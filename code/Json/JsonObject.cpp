#include "Core/Io/OutputStream.h"
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

void JsonObject::setMemberValue(const std::wstring& name, const Any& value)
{
	JsonMember* member = getMember(name);
	if (member)
		member->setValue(value);
	else
		push(new JsonMember(name, value));
}

Any JsonObject::getMemberValue(const std::wstring& name)
{
	JsonMember* member = getMember(name);
	return member ? member->getValue() : Any();
}

bool JsonObject::write(OutputStream& os) const
{
	os << L"{" << Endl;
	os << IncreaseIndent;

	for (RefArray< JsonMember >::const_iterator i = m_members.begin(); i != m_members.end(); ++i)
	{
		if (i != m_members.begin())
			os << L"," << Endl;

		if (!(*i)->write(os))
			return false;
	}
	if (!m_members.empty())
		os << Endl;

	os << DecreaseIndent;
	os << L"}" << Endl;
	return true;
}

	}
}
