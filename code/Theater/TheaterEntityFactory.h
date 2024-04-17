/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "World/AbstractEntityFactory.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_THEATER_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::theater
{

/*!
 * \ingroup Theater
 */
class T_DLLCLASS TheaterEntityFactory : public world::AbstractEntityFactory
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getWorldComponentTypes() const override final;

	virtual Ref< world::IWorldComponent > createWorldComponent(const world::IEntityBuilder* builder, const world::IWorldComponentData& worldComponentData) const override final;
};

}
