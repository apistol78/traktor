#include <cctype>
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Array", Array, ActionObject)

Array::Array()
:	ActionObject("Array")
{
}

Ref< Array > Array::concat() const
{
	Ref< Array > out = new Array();
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	return out;
}

Ref< Array > Array::concat(const ActionValueArray& values) const
{
	Ref< Array > out = new Array();
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	out->m_values.insert(out->m_values.end(), &values[0], &values[values.size() - 1]);
	return out;
}

std::string Array::join(const std::string& delimiter) const
{
	std::stringstream ss;
	if (m_values.size() >= 1)
	{
		ss << m_values[0].getString();
		for (uint32_t i = 1; i < uint32_t(m_values.size()); ++i)
			ss << delimiter << m_values[i].getString();
	}
	return ss.str();
}

void Array::push(const ActionValue& value)
{
	m_values.push_back(value);
}

ActionValue Array::pop()
{
	if (!m_values.empty())
	{
		ActionValue out = m_values.back();
		m_values.pop_back();
		return out;
	}
	else
		return ActionValue();
}

void Array::reverse()
{
	std::reverse(m_values.begin(), m_values.end());
}

ActionValue Array::shift()
{
	if (!m_values.empty())
	{
		ActionValue out = m_values.front();
		m_values.erase(m_values.begin());
		return out;
	}
	else
		return ActionValue();
}

uint32_t Array::unshift(const ActionValueArray& values)
{
	std::vector< ActionValue > head(values.size());
	for (uint32_t i = 0; i < values.size(); ++i)
		head[i] = values[i];
	m_values.insert(m_values.begin(), head.begin(), head.end());
	return uint32_t(m_values.size());
}

Ref< Array > Array::slice(int32_t startIndex, int32_t endIndex) const
{
	if (startIndex < 0)
		startIndex = int32_t(m_values.size() + startIndex);
	if (endIndex < 0)
		endIndex = int32_t(m_values.size() + endIndex);

	Ref< Array > copy = new Array();
	for (int32_t i = startIndex; i < endIndex; ++i)
	{
		if (i >= 0 && i < int32_t(m_values.size()))
			copy->m_values.push_back(m_values[i]);
	}

	return copy;
}

Ref< Array > Array::splice(int32_t startIndex, uint32_t deleteCount, const ActionValueArray& values, int32_t offset)
{
	if (startIndex < 0)
		startIndex = int32_t(m_values.size() + startIndex);

	Ref< Array > removed = new Array();
	while (deleteCount > 0)
	{
		if (startIndex >= int32_t(m_values.size()))
			break;
		removed->m_values.push_back(m_values[startIndex]);
		m_values.erase(m_values.begin() + startIndex);
		deleteCount--;
	}

	if (startIndex < int32_t(m_values.size()))
	{
		for (uint32_t i = offset; i < values.size(); ++i)
			m_values.insert(m_values.begin() + startIndex, values[i]);
	}
	else
	{
		for (uint32_t i = offset; i < values.size(); ++i)
			m_values.push_back(values[i]);
	}

	return removed;
}

uint32_t Array::length() const
{
	return uint32_t(m_values.size());
}

void Array::setMember(const ActionValue& memberName, const ActionValue& memberValue)
{
	if (memberName.isNumeric())
	{
		int32_t index = int32_t(memberName.getNumber());
		if (index >= 0 && index < int32_t(m_values.size()))
		{
			m_values[index] = memberValue;
			return;
		}
	}
	ActionObject::setMember(memberName, memberValue);
}

bool Array::getMember(ActionContext* context, const ActionValue& memberName, ActionValue& outMemberValue)
{
	if (memberName.isNumeric())
	{
		int32_t index = int32_t(memberName.getNumber());
		if (index >= 0 && index < int32_t(m_values.size()))
		{
			outMemberValue = m_values[index];
			return true;
		}
	}
	return ActionObject::getMember(context, memberName, outMemberValue);
}

ActionValue Array::toString() const
{
	StringOutputStream ss;
	for (std::vector< ActionValue >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
	{
		if (i != m_values.begin())
			ss << L", ";
		ss << i->getWideString();
	}
	return ActionValue(ss.str());
}

void Array::trace(const IVisitor& visitor) const
{
	for (std::vector< ActionValue >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
	{
		if (i->isObject())
			visitor(i->getObjectUnsafe());
	}
	ActionObject::trace(visitor);
}

void Array::dereference()
{
	m_values.clear();
	ActionObject::dereference();
}

	}
}

