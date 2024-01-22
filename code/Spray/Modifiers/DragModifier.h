/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Spray/Modifier.h"

namespace traktor::spray
{

/*! Drag modifier.
 * \ingroup Spray
 */
class DragModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	explicit DragModifier(float linearDrag, float angularDrag);

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Scalar m_linearDrag;
	float m_angularDrag;
};

}
