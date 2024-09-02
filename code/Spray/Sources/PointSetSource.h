/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Math/Range.h"
#include "Resource/Proxy.h"
#include "Spray/Source.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::spray
{

class PointSet;

/*! Point set particle source.
 * \ingroup Spray
 */
class T_DLLCLASS PointSetSource : public Source
{
	T_RTTI_CLASS;

public:
	explicit PointSetSource(
		float constantRate,
		float velocityRate,
		const resource::Proxy< PointSet >& pointSet,
		const Vector4& offset,
		const Range< float >& velocity,
		const Range< float >& orientation,
		const Range< float >& angularVelocity,
		const Range< float >& age,
		const Range< float >& mass,
		const Range< float >& size
	);

	virtual void emit(
		Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		EmitterInstanceCPU& emitterInstance
	) const override final;

	const resource::Proxy< PointSet >& getPointSet() const { return m_pointSet; }

	const Vector4& getOffset() const { return m_offset; }

private:
	resource::Proxy< PointSet > m_pointSet;
	Vector4 m_offset;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

}
