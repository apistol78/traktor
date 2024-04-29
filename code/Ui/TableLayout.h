/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Containers/AlignedVector.h"
#include "Ui/Layout.h"
#include "Ui/Size.h"
#include "Ui/Unit.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Table layout.
 * \ingroup UI
 */
class T_DLLCLASS TableLayout : public Layout
{
	T_RTTI_CLASS;

public:
	explicit TableLayout(const std::wstring& cdef, const std::wstring& rdef, Unit margin, Unit pad);

	virtual bool fit(Widget* widget, const Size& bounds, Size& result) override;

	virtual void update(Widget* widget) override;

private:
	Unit m_margin;
	Unit m_pad;
	AlignedVector< int32_t > m_cdef;
	AlignedVector< int32_t > m_rdef;
};

}
