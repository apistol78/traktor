/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <limits>
#include "Ui/EditValidator.h"

#undef min
#undef max

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_UI_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::ui
{

/*! Numeric text edit validator.
 * \ingroup UI
 */
class T_DLLCLASS NumericEditValidator : public EditValidator
{
	T_RTTI_CLASS;

public:
	explicit NumericEditValidator(
		bool floatPoint,
		float min = -std::numeric_limits< float >::max(),
		float max = std::numeric_limits< float >::max(),
		int32_t decimals = 8
	);

	virtual EditValidator::Result validate(const std::wstring& text) const override;

private:
	bool m_floatPoint;
	float m_min;
	float m_max;
	int32_t m_decimals;
};

}
