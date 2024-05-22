/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/AlignedVector.h"
#include "Ui/Widget.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Static text.
 * \ingroup UI
 */
class T_DLLCLASS Static : public Widget
{
	T_RTTI_CLASS;

public:
	bool create(Widget* parent, const std::wstring& text = L"");

	virtual void setText(const std::wstring& text) override;

	virtual Size getPreferredSize(const Size& hint) const override;

	virtual Size getMaximumSize() const override;

private:
	AlignedVector< std::pair< int32_t, std::wstring > > m_lines;
	Size m_extent = Size(0, 0);

	void eventPaint(PaintEvent* event);
};

}
