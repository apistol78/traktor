/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/RefArray.h"
#include "Webber/Event.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

/*! Tree view item; children are only built when expanded.
 * \ingroup Webber
 */
class TreeViewItem : public Widget
{
	T_RTTI_CLASS;

public:
	explicit TreeViewItem(const std::wstring& text);

	void setText(const std::wstring& text);

	const std::wstring& getText() const;

	void setExpanded(bool expanded);

	bool isExpanded() const;

	TreeViewItem* addChild(TreeViewItem* child);

	void removeChild(TreeViewItem* child);

	const RefArray< TreeViewItem >& getChildren() const;

	Event& clicked();

	virtual void build(html::Element* parent) const override final;

	virtual void consume(int32_t senderId, int32_t action) override final;

private:
	std::wstring m_text;
	bool m_expanded = false;
	RefArray< TreeViewItem > m_children;
	Event m_clicked;
};

}
