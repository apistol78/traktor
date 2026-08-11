/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Containers/StaticVector.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::sound
{

class T_DLLCLASS SurroundEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	typedef StaticVector< Transform, 4 > listenerTransformVector_t;

	explicit SurroundEnvironment(
		float maxDistance,
		float innerRadius,
		float fallOffExponent,
		float verticalScale,
		bool fullSurround
	);

	void setMaxDistance(float maxDistance);

	const Scalar& getMaxDistance() const { return m_maxDistance; }

	void setInnerRadius(float innerRadius);

	const Scalar& getInnerRadius() const { return m_innerRadius; }

	void setFallOffExponent(float fallOffExponent);

	const Scalar& getFallOffExponent() const { return m_fallOffExponent;  }

	void setVerticalScale(float verticalScale);

	float getVerticalScale() const { return m_verticalAxisScale.y(); }

	void setFullSurround(bool fullSurround);

	bool getFullSurround() const { return m_fullSurround; }

	void setListenerTransforms(const listenerTransformVector_t& listenerTransforms);

	const listenerTransformVector_t& getListenerTransforms() const { return m_listenerTransforms; }

	/*! Length of a listener relative vector, with the world vertical axis scaled.
	 *
	 * A vertical scale above one make height count for more than the same
	 * distance along the ground, so a source a floor below is heard as being
	 * further away than one the same number of meters to the side.
	 *
	 * \param delta Vector from listener to source, in world space.
	 * \return Scaled distance.
	 */
	Scalar getScaledDistance(const Vector4& delta) const { return (delta * m_verticalAxisScale).length(); }

private:
	Scalar m_maxDistance;
	Scalar m_innerRadius;
	Scalar m_fallOffExponent;
	Vector4 m_verticalAxisScale;
	bool m_fullSurround;
	listenerTransformVector_t m_listenerTransforms;
};

}
