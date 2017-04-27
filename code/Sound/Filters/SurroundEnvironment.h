/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_sound_SurroundEnvironment_H
#define traktor_sound_SurroundEnvironment_H

#include "Core/Object.h"
#include "Core/Math/Transform.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace sound
	{

class T_DLLCLASS SurroundEnvironment : public Object
{
	T_RTTI_CLASS;

public:
	SurroundEnvironment(
		float maxDistance,
		float innerRadius,
		float fallOffExponent,
		bool fullSurround
	);

	void setMaxDistance(float maxDistance);

	const Scalar& getMaxDistance() const { return m_maxDistance; }

	void setInnerRadius(float innerRadius);

	const Scalar& getInnerRadius() const { return m_innerRadius; }

	void setFallOffExponent(float fallOffExponent);

	const Scalar& getFallOffExponent() const { return m_fallOffExponent;  }

	void setFullSurround(bool fullSurround);

	bool getFullSurround() const { return m_fullSurround; }

	void setListenerTransform(const Transform& listenerTransform);

	const Transform& getListenerTransform() const { return m_listenerTransform; }

	const Transform& getListenerTransformInv() const { return m_listenerTransformInv; }

private:
	Scalar m_maxDistance;
	Scalar m_innerRadius;
	Scalar m_fallOffExponent;
	bool m_fullSurround;
	Transform m_listenerTransform;
	Transform m_listenerTransformInv;
};

	}
}

#endif	// traktor_sound_SurroundEnvironment_H
