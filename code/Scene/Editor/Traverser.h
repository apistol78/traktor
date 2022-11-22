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
#include "Core/Ref.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCENE_EDITOR_EXPORT)
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

class EntityData;

}

namespace traktor::scene
{

/*! Scene/entity data traverser.
 * \ingroup Scene
 */
class T_DLLCLASS Traverser
{
public:
	enum VisitorResult
	{
		VrContinue,
		VrSkip,
		VrFailed
	};

	static bool visit(const ISerializable* object, const std::function< VisitorResult(const world::EntityData*) >& visitor);

	static bool visit(ISerializable* object, const std::function< VisitorResult(Ref< world::EntityData >&) >& visitor);
};

}
