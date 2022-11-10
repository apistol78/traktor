/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

/*! Persistent hierarchical view state.
 * \ingroup UI
 */
class T_DLLCLASS HierarchicalState : public ISerializable
{
	T_RTTI_CLASS;

public:
	void setScrollPosition(int32_t scrollPosition);

	int32_t getScrollPosition() const;

	void addState(const std::wstring& path, bool expanded, bool selected);

	bool getExpanded(const std::wstring& path) const;

	bool getSelected(const std::wstring& path) const;

	/*! Merge tree states.
	 *
	 * Right-hand state override existing states;
	 * thus if a state exist in both left- and right-hand
	 * state right-hand is used.
	 *
	 * \param state Right hand state.
	 * \return Merged state.
	 */
	Ref< HierarchicalState > merge(const HierarchicalState* state) const;

	virtual void serialize(ISerializer& s) override;

private:
	int32_t m_scrollPosition = 0;
	std::map< std::wstring, std::pair< bool, bool > > m_states;
};

	}
}

