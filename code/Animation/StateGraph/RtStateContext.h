/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Core/Object.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_ANIMATION_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::animation
{

/*! State evaluation context.
 * \ingroup Animation
 */
class T_DLLCLASS RtStateContext : public Object
{
	T_RTTI_CLASS;

public:
	bool setParameterValue(const render::Handle& handle, bool value);

	bool getParameterValue(const render::Handle& handle) const;

	const AlignedVector< bool > getValues() const { return m_values; }

private:
	friend class RtStateGraphData;

	SmallMap< render::Handle, int32_t > m_parameters;
	AlignedVector< bool > m_values;
};

}
