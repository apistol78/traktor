/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Class/IObjectInspector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::world
{

/*!
 */
class T_DLLCLASS EntityObjectInspector : public IObjectInspector
{
	T_RTTI_CLASS;

public:
	virtual bool supportType(const TypeInfo& objectType) const override final;

	virtual std::wstring toString(const ITypedObject* object) const override final;
};

}
