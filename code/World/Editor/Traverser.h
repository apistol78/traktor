/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
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

class EntityData;

/*! Entity data traverser.
 * \ingroup World
 */
class T_DLLCLASS Traverser
{
public:
	enum class Result
	{
		Continue,
		Skip,
		Failed
	};

	static bool visit(const ISerializable* object, const std::function< Result (const EntityData*) >& visitor);

	static bool visit(ISerializable* object, const std::function< Result (Ref< EntityData >&) >& visitor);
};

}
