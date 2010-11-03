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
		float maxDistance = 10.0f,
		float innerRadius = 1.0f,
		bool fullSurround = true
	);

	void setMaxDistance(float maxDistance);

	const Scalar& getMaxDistance() const { return m_maxDistance; }

	void setInnerRadius(float innerRadius);

	const Scalar& getInnerRadius() const { return m_innerRadius; }

	void setFullSurround(bool fullSurround);

	bool getFullSurround() const { return m_fullSurround; }

	void setListenerTransform(const Transform& listenerTransform);

	const Transform& getListenerTransform() const { return m_listenerTransform; }

	const Transform& getListenerTransformInv() const { return m_listenerTransformInv; }

private:
	Scalar m_maxDistance;
	Scalar m_innerRadius;
	bool m_fullSurround;
	Transform m_listenerTransform;
	Transform m_listenerTransformInv;
};

	}
}

#endif	// traktor_sound_SurroundEnvironment_H
