/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Object.h"
#include "Ui/Size.h"

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

class IFontMetric;

/*! Font metrics.
 * \ingroup UI
 */
class T_DLLCLASS FontMetric : public Object
{
	T_RTTI_CLASS;

public:
	explicit FontMetric(const IFontMetric* metric);

	/*! Distance from top to base line. */
	int32_t getAscent() const;

	/*! Distance from base line to bottom. */
	int32_t getDescent() const;

	/*! Distance from top to bottom. */
	int32_t getHeight() const;

	/*! Advancement to next character. */
	int32_t getAdvance(wchar_t ch, wchar_t next) const;

	/*! Line spacing. */
	int32_t getLineSpacing() const;

	/*! Extent of text. */
	Size getExtent(const std::wstring& text) const;

private:
	const IFontMetric* m_metric;
};

	}
}
