/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SHAPE_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::shape
{

class IProbe;

class T_DLLCLASS TracerEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	explicit TracerEnvironment(const IProbe* environment);

	const IProbe* getEnvironment() const { return m_environment; }

private:
	Ref< const IProbe > m_environment;
};

}