#include "Ui/TreeViewState.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/MemberStl.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.ui.TreeViewState", TreeViewState, Serializable)

void TreeViewState::addState(const std::wstring& path, bool expanded, bool selected)
{
	if (expanded || selected)
		m_states.insert(std::make_pair(path, std::make_pair(expanded, selected)));
	else
	{
		std::map< std::wstring, std::pair< bool, bool > >::iterator i = m_states.find(path);
		if (i != m_states.end())
			m_states.erase(i);
	}
}

bool TreeViewState::getExpanded(const std::wstring& path) const
{
	std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = m_states.find(path);
	return i != m_states.end() ? i->second.first : false;
}

bool TreeViewState::getSelected(const std::wstring& path) const
{
	std::map< std::wstring, std::pair< bool, bool > >::const_iterator i = m_states.find(path);
	return i != m_states.end() ? i->second.second : false;
}

bool TreeViewState::serialize(Serializer& s)
{
	return s >> MemberStlMap< 
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
