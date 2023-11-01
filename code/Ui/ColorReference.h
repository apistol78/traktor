/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include "Core/Math/Color4ub.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class Object;

}

namespace traktor::ui
{

class StyleSheet;

/*! Color reference
 * \ingroup UI
 */
class T_DLLCLASS ColorReference
{
public:
	ColorReference() = default;

	ColorReference(const Object* ref, const std::wstring& style);

	ColorReference(const Color4ub& xcolor);

	Color4ub resolve(const StyleSheet* styleSheet) const;

	operator bool () const;

private:
	const Object* m_ref = nullptr;
	std::wstring m_style;
	Color4ub m_xcolor = Color4ub(0, 0, 0, 0);
};

}
