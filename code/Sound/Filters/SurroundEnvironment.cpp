#include "Sound/Filters/SurroundEnvironment.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundEnvironment", SurroundEnvironment, Object)

SurroundEnvironment::SurroundEnvironment(
	float maxDistance,
	float innerRadius
)
:	m_maxDistance(maxDistance)
,	m_innerRadius(innerRadius)
,	m_listenerTransform(Matrix44::identity())
,	m_listenerTransformInv(Matrix44::identity())
{
}

void SurroundEnvironment::setMaxDistance(float maxDistance)
{
	m_maxDistance = maxDistance;
}

void SurroundEnvironment::setInnerRadius(float innerRadius)
{
	m_innerRadius = innerRadius;
}

void SurroundEnvironment::setListenerTransform(const Matrix44& listenerTransform)
{
	m_listenerTransform = listenerTransform;
	m_listenerTransformInv = listenerTransform.inverse();
}

	}
}
