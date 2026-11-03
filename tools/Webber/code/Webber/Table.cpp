/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Html/Element.h"
#include "Html/Text.h"
#include "Webber/Table.h"
#include "Webber/Widget.h"

namespace traktor::wbr
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.wbr.Table", Table, ILayout)

Table::Table(const std::vector< std::wstring >& headers)
:	m_headers(headers)
{
}

void Table::build(const Widget* widget, const RefArray< Widget >& children, html::Element* parent) const
{
	auto etable = new html::Element(L"table");
	etable->setAttribute(L"class", L"wtk_layout_table");
	parent->addChild(etable);

	auto etr = new html::Element(L"tr");
	etable->addChild(etr);
	for (const auto& header : m_headers)
	{
		auto eth = new html::Element(L"th");
		eth->addChild(new html::Text(header));
		etr->addChild(eth);
	}

	const size_t columns = m_headers.size();
	for (size_t i = 0; i < children.size(); ++i)
	{
		if (i % columns == 0)
		{
			etr = new html::Element(L"tr");
			etable->addChild(etr);
		}

		auto etd = new html::Element(L"td");
		children[i]->build(etd);
		etr->addChild(etd);
	}
}

}
