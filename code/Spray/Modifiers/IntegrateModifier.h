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

/*! Integrate particle velocity modifier.
 * \ingroup Spray
 */
class IntegrateModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	static constexpr int32_t OperationCode = 1;

	explicit IntegrateModifier(float timeScale, bool linear, bool angular);

	virtual void writeSequence(Vector4*& inoutSequence) const override final;

	virtual void update(const Scalar& deltaTime, const Transform& transform, pointVector_t& points, size_t first, size_t last) const override final;

private:
	Scalar m_timeScale;
	bool m_linear;
	bool m_angular;
};

}
