#include "Flash/Action/ActionArray.h"
#include "Flash/Action/Classes/AsArray.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Misc/String.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.ActionArray", ActionArray, ActionObject)

ActionArray::ActionArray()
:	ActionObject(AsArray::getInstance())
{
}

Ref< ActionArray > ActionArray::concat() const
{
	Ref< ActionArray > out = new ActionArray();
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	return out;
}

Ref< ActionArray > ActionArray::concat(const std::vector< ActionValue >& values) const
{
	Ref< ActionArray > out = new ActionArray();
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	out->m_values.insert(out->m_values.end(), values.begin(), values.end());
	return out;
}

std::wstring ActionArray::join(const std::wstring& delimiter) const
{
	StringOutputStream ss;
	if (m_values.size() >= 1)
	{
		ss << m_values[0].getStringSafe();
		for (uint32_t i = 1; i < uint32_t(m_values.size()); ++i)
			ss << delimiter << m_values[i].getStringSafe();
	}
	return ss.str();
}

void ActionArray::push(const ActionValue& value)
{
	m_values.push_back(value);
}

ActionValue ActionArray::pop()
{
	ActionValue out = m_values.back(); m_values.pop_back();
	return out;
}

uint32_t ActionArray::length() const
{
	return uint32_t(m_values.size());
}

void ActionArray::setMember(const std::wstring& memberName, const ActionValue& memberValue)
{
	int32_t index = parseString< int32_t >(memberName);
	if (index >= 0 && index < int32_t(m_values.size()))
		m_values[index] = memberValue;
	else
		ActionObject::setMember(memberName, memberValue);
}

bool ActionArray::getMember(const std::wstring& memberName, ActionValue& outMemberValue) const
{
	int32_t index = parseString< int32_t >(memberName);
	if (index >= 0 && index < int32_t(m_values.size()))
	{
		outMemberValue = m_values[index];
		return true;
	}
	return ActionObject::getMember(memberName, outMemberValue);
}

std::wstring ActionArray::toString() const
{
	StringOutputStream ss;
	for (std::vector< ActionValue >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
	{
		if (!ss.empty())
			ss << L", ";
		ss << i->getStringSafe();
	}
	return ss.str();
}

	}
}

