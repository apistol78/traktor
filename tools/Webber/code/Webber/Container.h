/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/RefArray.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

class ILayout;

/*! Container of child widgets, arranged by an optional layout.
 * \ingroup Webber
 */
class Container : public Widget
{
	T_RTTI_CLASS;

public:
	void setLayout(ILayout* layout);

	template < typename WidgetType >
	WidgetType* addChild(WidgetType* child) { m_children.push_back(child); return child; }

	void removeChild(Widget* child);

	void removeAllChildren();

	const RefArray< Widget >& getChildren() const;

	virtual void build(html::Element* parent) const override;

	virtual void consumeValues(const std::map< int32_t, std::wstring >& values) override;

	virtual void consume(int32_t senderId, int32_t action) override;

private:
	Ref< ILayout > m_layout;
	RefArray< Widget > m_children;
};

}
