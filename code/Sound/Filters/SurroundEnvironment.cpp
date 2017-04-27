/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Sound/Filters/SurroundEnvironment.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SurroundEnvironment", SurroundEnvironment, Object)

SurroundEnvironment::SurroundEnvironment(
	float maxDistance,
	float innerRadius,
	float fallOffExponent,
	bool fullSurround
)
:	m_maxDistance(maxDistance)
,	m_innerRadius(innerRadius)
,	m_fallOffExponent(fallOffExponent)
,	m_fullSurround(fullSurround)
,	m_listenerTransform(Transform::identity())
,	m_listenerTransformInv(Transform::identity())
{
}

void SurroundEnvironment::setMaxDistance(float maxDistance)
{
	m_maxDistance = Scalar(maxDistance);
}

void SurroundEnvironment::setInnerRadius(float innerRadius)
{
	m_innerRadius = Scalar(innerRadius);
}

void SurroundEnvironment::setFallOffExponent(float fallOffExponent)
{
	m_fallOffExponent = Scalar(fallOffExponent);
}

void SurroundEnvironment::setFullSurround(bool fullSurround)
{
	m_fullSurround = fullSurround;
}

void SurroundEnvironment::setListenerTransform(const Transform& listenerTransform)
{
	m_listenerTransform = listenerTransform;
	m_listenerTransformInv = listenerTransform.inverse();
}

	}
}
