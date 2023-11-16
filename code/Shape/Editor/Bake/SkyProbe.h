/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Shape/Editor/Bake/IProbe.h"

namespace traktor::shape
{
	
class SkyProbe : public IProbe
{
	T_RTTI_CLASS;

public:
	SkyProbe() = default;

	explicit SkyProbe(const Vector4& sunDirection);

	virtual Color4f sampleRadiance(const Vector4& direction) const override final;

	virtual void serialize(ISerializer& s) override final;

private:
	Vector4 m_sunDirection;
};
	
}
