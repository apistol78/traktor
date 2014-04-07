#include <cctype>
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Flash/Action/Classes/Array.h"

namespace traktor
{
	namespace flash
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.flash.Array", Array, ActionObjectRelay)

Array::Array(uint32_t capacity)
:	ActionObjectRelay("Array")
{
	m_values.reserve(capacity);
}

Array::Array(const ActionValueArray& values)
:	ActionObjectRelay("Array")
{
	m_values.resize(values.size());
	for (uint32_t i = 0; i < values.size(); ++i)
		m_values[i] = values[i];
}

Ref< Array > Array::concat() const
{
	Ref< Array > out = new Array(uint32_t(m_values.size()));
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	return out;
}

Ref< Array > Array::concat(const ActionValueArray& values) const
{
	Ref< Array > out = new Array(uint32_t(m_values.size() + values.size()));
	out->m_values.insert(out->m_values.end(), m_values.begin(), m_values.end());
	for (uint32_t i = 0; i < values.size(); ++i)
		out->m_values.push_back(values[i]);
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
	AlignedVector< ActionValue > head(values.size());
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

	if (startIndex <= endIndex)
	{
		Ref< Array > copy = new Array(endIndex - startIndex);
		for (int32_t i = startIndex; i < endIndex; ++i)
		{
			if (i >= 0 && i < int32_t(m_values.size()))
				copy->m_values.push_back(m_values[i]);
		}
		return copy;
	}
	else
		return 0;
}

Ref< Array > Array::splice(int32_t startIndex, uint32_t deleteCount, const ActionValueArray& values, int32_t offset)
{
	if (startIndex < 0)
		startIndex = int32_t(m_values.size() + startIndex);

	Ref< Array > removed = new Array(deleteCount);
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

int32_t Array::indexOf(const ActionValue& value) const
{
	for (int32_t i = 0; i < int32_t(m_values.size()); ++i)
	{
		if (m_values[i] == value)
			return i;
	}
	return -1;
}

void Array::removeAt(int32_t index)
{
	m_values.erase(m_values.begin() + index);
}

void Array::removeAll()
{
	m_values.clear();
}

void Array::trace(const IVisitor& visitor) const
{
	for (AlignedVector< ActionValue >::const_iterator i = m_values.begin(); i != m_values.end(); ++i)
	{
		if (i->isObjectStrong())
			visitor(i->getObject());
	}
	ActionObjectRelay::trace(visitor);
}

void Array::dereference()
{
	m_values.clear();
	ActionObjectRelay::dereference();
}

	}
}

