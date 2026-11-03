/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/RefArray.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

class TreeViewItem;

/*! Tree view.
 * \ingroup Webber
 */
class TreeView : public Widget
{
	T_RTTI_CLASS;

public:
	TreeViewItem* addChild(TreeViewItem* child);

	void removeChild(TreeViewItem* child);

	const RefArray< TreeViewItem >& getChildren() const;

	virtual void build(html::Element* parent) const override final;

	virtual void consume(int32_t senderId, int32_t action) override final;

private:
	RefArray< TreeViewItem > m_children;
};

}
