/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/SmallMap.h"
#include "Input/InputTypes.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_INPUT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::input
{

/*!
 * \ingroup Input
 */
class T_DLLCLASS InputValueSet : public Object
{
	T_RTTI_CLASS;

public:
	void set(handle_t valueId, float value);

	float get(handle_t valueId) const;

private:
	SmallMap< handle_t, float > m_valueMap;
};

}
