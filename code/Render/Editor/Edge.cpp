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

namespace traktor
{
	namespace render
	{
		namespace
		{

struct OutputPinAccessor
{
	static const OutputPin* get(Ref< Node >& node, const std::wstring& name) {
		return node->findOutputPin(name);
	}
};

struct InputPinAccessor
{
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
			Ref< Node > node = m_pin ? m_pin->getNode() : 0;
			std::wstring name = m_pin ? m_pin->getName() : L"";

			s >> MemberRef< Node >(L"node", node);
			s >> Member< std::wstring >(L"name", name);
		}
		else	// Direction::Read
		{
			Ref< Node > node;
			std::wstring name;

			s >> MemberRef< Node >(L"node", node);
			s >> Member< std::wstring >(L"name", name);

			if (node)
				m_pin = PinAccessor::get(node, name);
			else
				m_pin = nullptr;
		}
	}

private:
	PinType*& m_pin;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.render.Edge", 0, Edge, ISerializable)

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
}
