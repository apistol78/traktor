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
#include "Core/Guid.h"
#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class ISerializable;

}

namespace traktor::world
{

/*! Resolve external entities, i.e. flatten scene without external references. */
Ref< ISerializable > T_DLLCLASS resolveExternal(
	const std::function< Ref< const ISerializable >(const Guid& objectId) >& getObjectFn,
	const ISerializable* object,
	const Guid& seed,
	AlignedVector< Guid >* outExternalEntities
);

}
