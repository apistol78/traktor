/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/String.h"
#include "Input/Binding/InCombine.h"
#include "Input/Editor/InCombineTraits.h"

namespace traktor
{
	namespace input
	{

std::wstring InCombineTraits::getHeader(const IInputNode* node) const
{
	return L"Combine";
}

std::wstring InCombineTraits::getDescription(const IInputNode* node) const
{
	const InCombine* inCombine = checked_type_cast< const InCombine*, false >(node);
	const auto& entries = inCombine->m_entries;

	wchar_t opc = L'?';
	switch (inCombine->m_operator)
	{
	case InCombine::CoAdd:
		opc = '+';
		break;

	case InCombine::CoSub:
		opc = '-';
		break;

	case InCombine::CoMul:
		opc = L'*';
		break;

	case InCombine::CoDiv:
		opc = L'/';
		break;

	default:
		break;
	}

	StringOutputStream ss;
	if (!entries.empty())
	{
		for (size_t i = 0; i < entries.size(); ++i)
		{
			if (i > 0)
				ss << L" " << opc << L" ";

			ss << L"([" << uint32_t(i) << L"]";

			if (entries[i].mul != 1.0f)
				ss << L" * " << entries[i].mul;

			if (entries[i].add != 0.0f)
				ss << L" + " << entries[i].add;

			ss << L")";
		}
	}
	else
		ss << opc;

	return ss.str();
}

Ref< IInputNode > InCombineTraits::createNode() const
{
	return new InCombine();
}

void InCombineTraits::getInputNodes(const IInputNode* node, std::map< const std::wstring, Ref< const IInputNode > >& outInputNodes) const
{
	const InCombine* inCombine = checked_type_cast< const InCombine*, false >(node);

	const auto& entries = inCombine->m_entries;
	for (uint32_t i = 0; i < (uint32_t)entries.size(); ++i)
		outInputNodes[toString(i)] = entries[i].source;

	outInputNodes[L"*"] = 0;
}

void InCombineTraits::connectInputNode(IInputNode* node, const std::wstring& inputName, IInputNode* sourceNode) const
{
	InCombine* inCombine = checked_type_cast< InCombine*, false >(node);
	if (inputName == L"*")
	{
		InCombine::Entry entry;
		entry.source = sourceNode;
		entry.mul = 1.0f;
		entry.add = 0.0f;
		inCombine->m_entries.push_back(entry);
	}
	else
	{
		const int32_t index = parseString< int32_t >(inputName);
		if (index >= 0 && index < (int32_t)inCombine->m_entries.size())
			inCombine->m_entries[index].source = sourceNode;
	}
}

void InCombineTraits::disconnectInputNode(IInputNode* node, const std::wstring& inputName) const
{
	InCombine* inCombine = checked_type_cast< InCombine*, false >(node);
	const int32_t index = parseString< int32_t >(inputName);
	if (index >= 0 && index < (int32_t)inCombine->m_entries.size())
		inCombine->m_entries.erase(inCombine->m_entries.begin() + index);
}

	}
}
