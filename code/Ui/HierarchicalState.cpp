/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Ui/HierarchicalState.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.HierarchicalState", 0, HierarchicalState, ISerializable)

void HierarchicalState::setScrollPosition(int32_t scrollPosition)
{
	m_scrollPosition = scrollPosition;
}

int32_t HierarchicalState::getScrollPosition() const
{
	return m_scrollPosition;
}

void HierarchicalState::addState(const std::wstring& path, bool expanded, bool selected)
{
	if (expanded || selected)
		m_states[path] = std::make_pair(expanded, selected);
	else
	{
		auto it = m_states.find(path);
		if (it != m_states.end())
			m_states.erase(it);
	}
}

bool HierarchicalState::getExpanded(const std::wstring& path) const
{
	auto it = m_states.find(path);
	return it != m_states.end() ? it->second.first : false;
}

bool HierarchicalState::getSelected(const std::wstring& path) const
{
	auto it = m_states.find(path);
	return it != m_states.end() ? it->second.second : false;
}

Ref< HierarchicalState > HierarchicalState::merge(const HierarchicalState* state) const
{
	Ref< HierarchicalState > merged = new HierarchicalState();
	merged->m_states = m_states;

	for (const auto& it : state->m_states)
		merged->addState(it.first, it.second.first, it.second.second);

	return merged;
}

void HierarchicalState::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"scrollPosition", m_scrollPosition);
	s >> MemberStlMap<
		std::wstring,
		std::pair< bool, bool >,
		Member< std::wstring >,
		MemberStlPair< bool, bool >
	>(L"states", m_states);
}

	}
}
