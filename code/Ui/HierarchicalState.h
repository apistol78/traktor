/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_HierarchicalState_H
#define traktor_ui_HierarchicalState_H

#include <map>
#include "Core/Serialization/ISerializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace ui
	{

/*! \brief Persistent hierarchical view state.
 * \ingroup UI
 */
class T_DLLCLASS HierarchicalState : public ISerializable
{
	T_RTTI_CLASS;

public:
	HierarchicalState();

	void setScrollPosition(int32_t scrollPosition);

	int32_t getScrollPosition() const;

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
	Ref< HierarchicalState > merge(const HierarchicalState* state) const;

	virtual void serialize(ISerializer& s) T_OVERRIDE;

private:
	int32_t m_scrollPosition;
	std::map< std::wstring, std::pair< bool, bool > > m_states;
};

	}
}

#endif	// traktor_ui_HierarchicalState_H
