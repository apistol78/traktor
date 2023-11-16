/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Color4f.h"
#include "Core/Serialization/ISerializable.h"

namespace traktor::shape
{
	
class IProbe : public ISerializable
{
	T_RTTI_CLASS;

public:
	virtual Color4f sampleRadiance(const Vector4& direction) const = 0;
};
	
}
