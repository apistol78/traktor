/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Render/Editor/Edge.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

namespace traktor::render
{
	namespace
	{

struct OutputPinAccessor
{
	static const OutputPin* get(Ref< Node >& node, const Guid& id) {
		return node->findOutputPin(id);
	}
	static const OutputPin* get(Ref< Node >& node, const std::wstring& name) {
		return node->findOutputPin(name);
	}
};

struct InputPinAccessor
{
	static const InputPin* get(Ref< Node >& node, const Guid& id) {
		return node->findInputPin(id);
	}
	static const InputPin* get(Ref< Node >& node, const std::wstring& name) {
		return node->findInputPin(name);
	}
};

template < typename PinType, typename PinAccessor >
class MemberPin : public MemberComplex
{
public:
	MemberPin(const wchar_t* const name, PinType*& pin)
	:	MemberComplex(name, true)
	,	m_pin(pin)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::Direction::Write)
		{
			Ref< Node > node = m_pin ? m_pin->getNode() : nullptr;
			s >> MemberRef< Node >(L"node", node);

			if (s.getVersion< Edge >() >= 1)
			{
				Guid id = m_pin ? m_pin->getId() : Guid();
				s >> Member< Guid >(L"id", id);
			}
			else
			{
				std::wstring name = m_pin ? m_pin->getName() : L"";
				s >> Member< std::wstring >(L"name", name);
			}
		}
		else	// Direction::Read
		{
			Ref< Node > node;
			s >> MemberRef< Node >(L"node", node);

			if (s.getVersion< Edge >() >= 1)
			{
				Guid id;
				s >> Member< Guid >(L"id", id);
				if (node)
					m_pin = PinAccessor::get(node, id);
				else
					m_pin = nullptr;
			}
			else
			{
				std::wstring name;
				s >> Member< std::wstring >(L"name", name);
				if (node)
					m_pin = PinAccessor::get(node, name);
				else
					m_pin = nullptr;
			}
		}
	}

private:
	PinType*& m_pin;
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Edge", 1, Edge, ISerializable)

Edge::Edge(const OutputPin* source, const InputPin* destination)
:	m_source(source)
,	m_destination(destination)
{
	T_ASSERT(source != nullptr);
	T_ASSERT(destination != nullptr);
}

void Edge::serialize(ISerializer& s)
{
	s >> MemberPin< const OutputPin, OutputPinAccessor >(L"source", m_source);
	s >> MemberPin< const InputPin, InputPinAccessor >(L"destination", m_destination);
}

}
