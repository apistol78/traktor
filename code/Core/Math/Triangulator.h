/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <functional>
#include "Core/Config.h"
#include "Core/Math/Vector2.h"
#include "Core/Math/Vector4.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! Triangulator.
 * \ingroup Core
 */
class T_DLLCLASS Triangulator
{
public:
	enum class Mode
	{
		Sequential = 0,
		Sorted = 1
	};

	typedef std::function< void(size_t index0, size_t index1, size_t index2) > fn_callback_t;

	void freeze(
		const AlignedVector< Vector2 >& points,
		Mode mode,
		const fn_callback_t& callback
	);

	void freeze(
		const AlignedVector< Vector4 >& points,
		const Vector4& normal,
		Mode mode,
		const fn_callback_t& callback
	);
};

}

