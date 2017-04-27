/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_spray_LineSource_H
#define traktor_spray_LineSource_H

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Range.h"
#include "Spray/Source.h"

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

/*! \brief Line particle source.
 * \ingroup Spray
 */
class T_DLLCLASS LineSource : public Source
{
	T_RTTI_CLASS;

public:
	LineSource(
		float constantRate,
		float velocityRate,
		const Vector4& startPosition,
		const Vector4& endPosition,
		int32_t segments,
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
		EmitterInstance& emitterInstance
	) const T_OVERRIDE T_FINAL;

	const Vector4& getStartPosition() const { return m_startPosition; }

	const Vector4& getEndPosition() const { return m_endPosition; }

	int32_t getSegments() const { return m_segments; }

private:
	Vector4 m_startPosition;
	Vector4 m_endPosition;
	int32_t m_segments;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_LineSource_H
