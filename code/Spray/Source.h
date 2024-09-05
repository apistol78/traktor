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
#include "Core/Math/Transform.h"
#include "Resource/Id.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::render
{

class ProgramParameters;

}

namespace traktor::spray
{

struct Context;
class EmitterInstanceCPU;

/*! Particle source.
 * \ingroup Spray
 */
class T_DLLCLASS Source : public Object
{
	T_RTTI_CLASS;

public:
	explicit Source(
		float constantRate,
		float velocityRate
	);

	virtual void setShaderParameters(render::ProgramParameters* pp) const {}

	virtual void emit(
		Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		EmitterInstanceCPU& emitterInstance
	) const = 0;

	float getConstantRate() const { return m_constantRate; }

	float getVelocityRate() const { return m_velocityRate; }

private:
	float m_constantRate;
	float m_velocityRate;
};

}
