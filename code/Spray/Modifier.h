/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Transform.h"
#include "Core/Object.h"
#include "Spray/Point.h"

namespace traktor::spray
{

/*! Emitter modifier.
 * \ingroup Spray
 */
class Modifier : public Object
{
	T_RTTI_CLASS;

public:
	virtual void writeSequence(Vector4*& inoutSequence) const {};

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const = 0;
};

}
