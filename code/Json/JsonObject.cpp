/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/OutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Misc/StringSplit.h"
#include "Json/JsonArray.h"
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

JsonMember* JsonObject::getMember(const std::wstring& name) const
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

Any JsonObject::getMemberValue(const std::wstring& name) const
{
	const JsonMember* member = getMember(name);
	return member ? member->getValue() : Any();
}

Any JsonObject::getValue(const std::wstring& path) const
{
	Any iter = Any::fromObject(const_cast< JsonObject* >(this));

	StringSplit< std::wstring > s(path, L".");
	for (StringSplit< std::wstring >::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		if (JsonObject* nodeObject = dynamic_type_cast< JsonObject* >(iter.getObject()))
		{
			JsonMember* member = nodeObject->getMember(*i);
			if (member)
				iter = member->getValue();
			else
				return Any();
		}
		else if (JsonArray* nodeArray = dynamic_type_cast< JsonArray* >(iter.getObject()))
		{
			uint32_t index = parseString< int32_t >(*i, ~0U);
			if (index < nodeArray->size())
				iter = nodeArray->get(index);
			else
				return Any();
		}
	}

	return iter;
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
