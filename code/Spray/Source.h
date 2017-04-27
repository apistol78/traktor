/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_Source_H
#define traktor_spray_Source_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

struct Context;
class EmitterInstance;

/*! \brief Particle source.
 * \ingroup Spray
 */
class T_DLLCLASS Source : public Object
{
	T_RTTI_CLASS;

public:
	Source(
		float constantRate,
		float velocityRate
	);

	virtual void emit(
		Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const = 0;

	float getConstantRate() const { return m_constantRate; }

	float getVelocityRate() const { return m_velocityRate; }

private:
	float m_constantRate;
	float m_velocityRate;
};

	}
}

#endif	// traktor_spray_Source_H
