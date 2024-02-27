/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Transform.h"

namespace traktor::sound
{

class ISoundPlayer;

}

namespace traktor::world
{

class Entity;

}

namespace traktor::spray
{

/*! Virtual source callback.
 * \ingroup Spray
 */
struct VirtualSourceCallback
{
	virtual ~VirtualSourceCallback() {}

	virtual void virtualSourceEmit(
		const class VirtualSource* source,
		struct Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		class EmitterInstance& emitterInstance
	) = 0;
};

/*! Effect context.
 * \ingroup Spray
 */
struct Context
{
	float deltaTime = 0.0f;
	RandomGeometry random;
	world::Entity* owner = nullptr;
	VirtualSourceCallback* virtualSourceCallback = nullptr;
};

}
