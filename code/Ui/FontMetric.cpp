/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/FontMetric.h"
#include "Ui/Itf/IFontMetric.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.FontMetric", FontMetric, Object)

FontMetric::FontMetric(const IFontMetric* metric)
:	m_metric(metric)
{
}

int32_t FontMetric::getAscent() const
{
	T_FATAL_ASSERT(m_metric != nullptr);

	int32_t ascent, descent;
	m_metric->getAscentAndDescent(ascent, descent);

	return ascent;
}

int32_t FontMetric::getDescent() const
{
	T_FATAL_ASSERT(m_metric != nullptr);

	int32_t ascent, descent;
	m_metric->getAscentAndDescent(ascent, descent);

	return descent;
}

int32_t FontMetric::getHeight() const
{
	T_FATAL_ASSERT(m_metric != nullptr);

	int32_t ascent, descent;
	m_metric->getAscentAndDescent(ascent, descent);

	return ascent + descent;
}

int32_t FontMetric::getAdvance(wchar_t ch, wchar_t next) const
{
	T_FATAL_ASSERT(m_metric != nullptr);
	return m_metric->getAdvance(ch, next);
}

int32_t FontMetric::getLineSpacing() const
{
	T_FATAL_ASSERT(m_metric != nullptr);
	return m_metric->getLineSpacing();
}

Size FontMetric::getExtent(const std::wstring& text) const
{
	T_FATAL_ASSERT(m_metric != nullptr);
	return m_metric->getExtent(text);
}

}
