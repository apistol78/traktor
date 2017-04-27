/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberStl.h"
#include "Ui/HierarchicalState.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ui.HierarchicalState", 0, HierarchicalState, ISerializable)

HierarchicalState::HierarchicalState()
:	m_scrollPosition(0)
{
}

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
		std::map< std::wstring, std::pair< bool, bool > >::iterator i = m_states.find(path);
		if (i != m_states.end())
			m_states.erase(i);
	}
}

bool HierarchicalState::getExpanded(const std::wstring& path) const
{
	std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = m_states.find(path);
	return i != m_states.end() ? i->second.first : false;
}

bool HierarchicalState::getSelected(const std::wstring& path) const
{
	std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = m_states.find(path);
	return i != m_states.end() ? i->second.second : false;
}

Ref< HierarchicalState > HierarchicalState::merge(const HierarchicalState* state) const
{
	Ref< HierarchicalState > merged = new HierarchicalState();

	merged->m_states = m_states;

	for (std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = state->m_states.begin(); i != state->m_states.end(); ++i)
		merged->addState(i->first, i->second.first, i->second.second);

	return merged;
}

void HierarchicalState::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"scrollPosition", m_scrollPosition);
	s >> MemberStlMap< 
		std::wstring,
		std::pair< bool, bool >,
		MemberStlPair< 
			std::wstring, 
			std::pair< bool, bool >,
			Member< std::wstring >,
			MemberStlPair< bool, bool >
		>
	>(L"states", m_states);
}

	}
}
