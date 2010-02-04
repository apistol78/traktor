#ifndef traktor_ui_TreeViewState_H
#define traktor_ui_TreeViewState_H

#include <map>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Tree view state.
 * \ingroup UI
 */
class T_DLLCLASS TreeViewState : public ISerializable
{
	T_RTTI_CLASS;

public:
	void addState(const std::wstring& path, bool expanded, bool selected);

	bool getExpanded(const std::wstring& path) const;

	bool getSelected(const std::wstring& path) const;

	/*! \brief Merge tree states.
	 *
	 * Right-hand state override existing states;
	 * thus if a state exist in both left- and right-hand
	 * state right-hand is used.
	 *
	 * \param state Right hand state.
	 * \return Merged state.
	 */
	Ref< TreeViewState > merge(const TreeViewState* state) const;

	virtual bool serialize(ISerializer& s);

private:
	std::map< std::wstring, std::pair< bool, bool > > m_states;
};

	}
}

#endif	// traktor_ui_TreeViewState_H
