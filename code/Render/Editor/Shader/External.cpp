/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/AttributeReadOnly.h"
#include "Core/Serialization/AttributeType.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComplex.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Core/Serialization/MemberSmallMap.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/OutputPin.h"
#include "Render/Editor/Shader/External.h"
#include "Render/Editor/Shader/Nodes.h"
#include "Render/Editor/Shader/ShaderGraph.h"

namespace traktor::render
{
	namespace
	{

class MemberInputPin : public MemberComplex
{
public:
	typedef InputPin* value_type;

	explicit MemberInputPin(const wchar_t* const name, Node* node, value_type& pin)
	:	MemberComplex(name, true)
	,	m_node(node)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::Direction::Write)
		{
			Guid id = m_pin->getId();
			std::wstring name = m_pin->getName();
			bool optional = m_pin->isOptional();

			if (s.getVersion() >= 2)
				s >> Member< Guid >(L"id", id);

			s >> Member< std::wstring >(L"name", name);

			if (s.getVersion() >= 1)
				s >> Member< bool >(L"optional", optional);
		}
		else	// Direction::Read
		{
			Guid id;
			std::wstring name = L"";
			bool optional = false;

			if (s.getVersion() >= 2)
				s >> Member< Guid >(L"id", id);

			s >> Member< std::wstring >(L"name", name);

			if (s.getVersion() >= 1)
				s >> Member< bool >(L"optional", optional);

			if (m_pin)
			{
				*m_pin = InputPin(
					m_node,
					id,
					name,
					optional
				);
			}
			else
			{
				m_pin = new InputPin(
					m_node,
					id,
					name,
					optional
				);
			}
		}
	}

private:
	Node* m_node;
	value_type& m_pin;
};

class MemberOutputPin : public MemberComplex
{
public:
	typedef OutputPin* value_type;

	explicit MemberOutputPin(const wchar_t* const name, Node* node, value_type& pin)
	:	MemberComplex(name, true)
	,	m_node(node)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::Direction::Write)
		{
			Guid id = m_pin->getId();
			std::wstring name = m_pin->getName();

			if (s.getVersion() >= 2)
				s >> Member< Guid >(L"id", id);

			s >> Member< std::wstring >(L"name", name);
		}
		else	// Direction::Read
		{
			Guid id;
			std::wstring name;

			if (s.getVersion() >= 2)
				s >> Member< Guid >(L"id", id);

			s >> Member< std::wstring >(L"name", name);

			if (m_pin)
			{
				*m_pin = OutputPin(
					m_node,
					id,
					name
				);
			}
			else
			{
				m_pin = new OutputPin(
					m_node,
					id,
					name
				);
			}
		}
	}

private:
	Node* m_node;
	value_type& m_pin;
};

template < typename PinMember >
class MemberPinArray : public MemberArray
{
public:
	typedef typename PinMember::value_type pin_type;
	typedef AlignedVector< pin_type > value_type;

	explicit MemberPinArray(const wchar_t* const name, Node* node, value_type& pins)
	:	MemberArray(name, &m_attribute)
	,	m_node(node)
	,	m_pins(pins)
	,	m_index(0)
	{
	}

	virtual void reserve(size_t size, size_t capacity) const
	{
		m_pins.reserve(capacity);
	}

	virtual size_t size() const
	{
		return m_pins.size();
	}

	virtual void read(ISerializer& s) const
	{
		if (m_index >= m_pins.size())
			m_pins.push_back(nullptr);
		s >> PinMember(L"item", m_node, m_pins[m_index++]);
	}

	virtual void write(ISerializer& s) const
	{
		if (s.ensure(m_index < m_pins.size()))
			s >> PinMember(L"item", m_node, m_pins[m_index++]);
	}

	virtual bool insert() const
	{
		return false;
	}

private:
	AttributePrivate m_attribute;
	Node* m_node;
	value_type& m_pins;
	mutable size_t m_index;
};

struct SortInputPinPredicate
{
	bool operator () (const InputPin* pl, const InputPin* pr) const
	{
		if (!pl->isOptional() && pr->isOptional())
			return true;

		if (pl->isOptional() && !pr->isOptional())
			return false;

		return pl->getName().compare(pr->getName()) < 0;
	}
};

struct SortOutputPinPredicate
{
	bool operator () (const OutputPin* pl, const OutputPin* pr) const
	{
		return pl->getName().compare(pr->getName()) < 0;
	}
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.External", 2, External, Node)

External::External(const Guid& fragmentGuid, const ShaderGraph* fragmentGraph)
:	m_fragmentGuid(fragmentGuid)
{
	for (auto fragmentNode : fragmentGraph->getNodes())
	{
		if (const InputPort* inputPort = dynamic_type_cast< const InputPort* >(fragmentNode))
		{
			const Guid& id = inputPort->getId();
			const std::wstring name = inputPort->getName();

			if (inputPort->isConnectable())
			{
				m_inputPins.push_back(new InputPin(
					this,
					id,
					name,
					inputPort->isOptional()
				));
			}

			if (inputPort->haveDefaultValue() && (inputPort->isOptional() || !inputPort->isConnectable()))
				m_values[name] = inputPort->getDefaultValue();
		}
		else if (const OutputPort* outputPort = dynamic_type_cast< const OutputPort* >(fragmentNode))
		{
			const Guid& id = outputPort->getId();
			const std::wstring name = outputPort->getName();

			m_outputPins.push_back(new OutputPin(
				this,
				id,
				name
			));
		}
	}

	// Sort pins lexicographically.
	std::sort(m_inputPins.begin(), m_inputPins.end(), SortInputPinPredicate());
	std::sort(m_outputPins.begin(), m_outputPins.end(), SortOutputPinPredicate());
}

External::~External()
{
	for (auto pin : m_inputPins)
		delete pin;

	for (auto pin : m_outputPins)
		delete pin;

	m_inputPins.clear();
	m_outputPins.clear();
}

void External::setFragmentGuid(const Guid& fragmentGuid)
{
	m_fragmentGuid = fragmentGuid;
}

const Guid& External::getFragmentGuid() const
{
	return m_fragmentGuid;
}

void External::setValue(const std::wstring& name, float value)
{
	m_values[name] = value;
}

float External::getValue(const std::wstring& name, float defaultValue) const
{
	auto it = m_values.find(name);
	return it != m_values.end() ? it->second : defaultValue;
}

bool External::haveValue(const std::wstring& name) const
{
	return m_values.find(name) != m_values.end();
}

void External::removeValue(const std::wstring& name)
{
	auto it = m_values.find(name);
	if (it != m_values.end())
		m_values.erase(it);
}

const InputPin* External::createInputPin(const Guid& id, const std::wstring& name, bool optional)
{
	InputPin* inputPin = new InputPin(this, id, name, optional);
	m_inputPins.push_back(inputPin);
	std::sort(m_inputPins.begin(), m_inputPins.end(), SortInputPinPredicate());
	return inputPin;
}

const OutputPin* External::createOutputPin(const Guid& id, const std::wstring& name)
{
	OutputPin* outputPin = new OutputPin(this, id, name);
	m_outputPins.push_back(outputPin);
	std::sort(m_outputPins.begin(), m_outputPins.end(), SortOutputPinPredicate());
	return outputPin;
}

void External::removeInputPin(const InputPin* inputPin)
{
	auto it = std::find(m_inputPins.begin(), m_inputPins.end(), inputPin);
	T_ASSERT(it != m_inputPins.end());
	delete *it; m_inputPins.erase(it);
}

void External::removeOutputPin(const OutputPin* outputPin)
{
	auto it = std::find(m_outputPins.begin(), m_outputPins.end(), outputPin);
	T_ASSERT(it != m_outputPins.end());
	delete *it; m_outputPins.erase(it);
}

std::wstring External::getInformation() const
{
	return m_fragmentGuid.format();
}

int External::getInputPinCount() const
{
	return int(m_inputPins.size());
}

const InputPin* External::getInputPin(int index) const
{
	T_ASSERT(index >= 0 && index < int(m_inputPins.size()));
	return m_inputPins[index];
}

int External::getOutputPinCount() const
{
	return int(m_outputPins.size());
}

const OutputPin* External::getOutputPin(int index) const
{
	T_ASSERT(index >= 0 && index < int(m_outputPins.size()));
	return m_outputPins[index];
}

void External::serialize(ISerializer& s)
{
	Node::serialize(s);

	s >> Member< Guid >(L"fragmentGuid", m_fragmentGuid, AttributeType(type_of< ShaderGraph >()) | AttributePrivate());
	s >> MemberPinArray< MemberInputPin >(L"inputPins", this, m_inputPins);
	s >> MemberPinArray< MemberOutputPin >(L"outputPins", this, m_outputPins);

	if (s.getVersion() >= 1)
		s >> MemberSmallMap< std::wstring, float >(L"values", m_values, AttributeReadOnly());
}

}
