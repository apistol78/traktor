/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeMultiLine.h"
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Render/Editor/InputPin.h"
#include "Render/Editor/Node.h"
#include "Render/Editor/OutputPin.h"

namespace traktor
{
	namespace render
	{

T_IMPLEMENT_RTTI_VERSION_CLASS(L"traktor.render.Node", 1, Node, ISerializable)

void Node::setId(const Guid& instanceId)
{
	m_id = instanceId;
}

const Guid& Node::getId() const
{
	return m_id;
}

void Node::setComment(const std::wstring& comment)
{
	m_comment = comment;
}

const std::wstring& Node::getComment() const
{
	return m_comment;
}

std::wstring Node::getInformation() const
{
	return L"";
}

void Node::setPosition(const std::pair< int, int >& position)
{
	m_position = position;
}

const std::pair< int, int >& Node::getPosition() const
{
	return m_position;
}

const InputPin* Node::findInputPin(const Guid& id) const
{
	if (id.isNull() || !id.isValid())
		return nullptr;

	int count = getInputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const InputPin* inputPin = getInputPin(i);
		T_ASSERT(inputPin);

		if (inputPin->getId() == id)
			return inputPin;
	}

	return nullptr;
}

const InputPin* Node::findInputPin(const std::wstring& name) const
{
	int count = getInputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const InputPin* inputPin = getInputPin(i);
		T_ASSERT(inputPin);

		if (inputPin->getName() == name)
			return inputPin;
	}
	return nullptr;
}

const OutputPin* Node::findOutputPin(const Guid& id) const
{
	if (id.isNull() || !id.isValid())
		return nullptr;

	int count = getOutputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const OutputPin* outputPin = getOutputPin(i);
		T_ASSERT(outputPin);

		if (outputPin->getId() == id)
			return outputPin;
	}

	return nullptr;
}

const OutputPin* Node::findOutputPin(const std::wstring& name) const
{
	int count = getOutputPinCount();
	for (int i = 0; i < count; ++i)
	{
		const OutputPin* outputPin = getOutputPin(i);
		T_ASSERT(outputPin);

		if (outputPin->getName() == name)
			return outputPin;
	}
	return nullptr;
}

void Node::serialize(ISerializer& s)
{
	if (s.getVersion< Node >() >= 1)
		s >> Member< Guid >(L"id", m_id, AttributePrivate());
	else if (s.getDirection() == ISerializer::Direction::Read)
		m_id = Guid();

	s >> Member< std::wstring >(L"comment", m_comment, AttributeMultiLine());
	s >> MemberStlPair< int, int >(L"position", m_position, AttributePrivate());
}

	}
}
