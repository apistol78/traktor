/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ColorReference.h"
#include "Ui/StyleSheet.h"

namespace traktor::ui
{

ColorReference::ColorReference(const Object* ref, const std::wstring& style)
:	m_ref(ref)
,	m_style(style)
{
}

ColorReference::ColorReference(const Color4ub& xcolor)
:	m_xcolor(xcolor)
{
}

Color4ub ColorReference::resolve(const StyleSheet* styleSheet) const
{
	if (m_ref == nullptr || m_style.empty())
		return m_xcolor;
	else
		return styleSheet->getColor(m_ref, m_style);
}

ColorReference::operator bool () const
{
	return (m_ref != nullptr && !m_style.empty()) || m_xcolor.a != 0;
}

}
