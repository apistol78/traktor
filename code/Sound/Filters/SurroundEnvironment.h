#ifndef traktor_sound_SurroundEnvironment_H
#define traktor_sound_SurroundEnvironment_H

#include "Core/Object.h"
#include "Core/Math/Matrix44.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SOUND_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
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
		float innerRadius = 1.0f
	);

	void setMaxDistance(float maxDistance);

	float getMaxDistance() const { return m_maxDistance; }

	void setInnerRadius(float innerRadius);

	float getInnerRadius() const { return m_innerRadius; }

	void setListenerTransform(const Matrix44& listenerTransform);

	const Matrix44& getListenerTransform() const { return m_listenerTransform; }

	const Matrix44& getListenerTransformInv() const { return m_listenerTransformInv; }

private:
	float m_maxDistance;
	float m_innerRadius;
	Matrix44 m_listenerTransform;
	Matrix44 m_listenerTransformInv;
};

	}
}

#endif	// traktor_sound_SurroundEnvironment_H
