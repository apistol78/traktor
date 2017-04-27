/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/Hinge2JointDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.Hinge2JointDesc", 0, Hinge2JointDesc, JointDesc)

Hinge2JointDesc::Hinge2JointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis1(0.0f, 1.0f, 0.0f, 0.0f)
,	m_axis2(1.0f, 0.0f, 0.0f, 0.0f)
,	m_lowStop(0.0f)
,	m_highStop(0.0f)
,	m_suspensionEnable(false)
,	m_suspensionDamping(0.0f)
,	m_suspensionStiffness(0.0f)
{
}

void Hinge2JointDesc::setAnchor(const Vector4& anchor)
{
	m_anchor = anchor;
}

const Vector4& Hinge2JointDesc::getAnchor() const
{
	return m_anchor;
}

void Hinge2JointDesc::setAxis1(const Vector4& axis)
{
	m_axis1 = axis;
}

const Vector4& Hinge2JointDesc::getAxis1() const
{
	return m_axis1;
}

void Hinge2JointDesc::setAxis2(const Vector4& axis)
{
	m_axis2 = axis;
}

const Vector4& Hinge2JointDesc::getAxis2() const
{
	return m_axis2;
}

void Hinge2JointDesc::setLowStop(float lowStop)
{
	m_lowStop = lowStop;
}

float Hinge2JointDesc::getLowStop() const
{
	return m_lowStop;
}

void Hinge2JointDesc::setHighStop(float highStop)
{
	m_highStop = highStop;
}

float Hinge2JointDesc::getHighStop() const
{
	return m_highStop;
}

void Hinge2JointDesc::setSuspensionEnable(bool suspensionEnable)
{
	m_suspensionEnable = suspensionEnable;
}

bool Hinge2JointDesc::getSuspensionEnable() const
{
	return m_suspensionEnable;
}

void Hinge2JointDesc::setSuspensionDamping(float suspensionDamping)
{
	m_suspensionDamping = suspensionDamping;
}

float Hinge2JointDesc::getSuspensionDamping() const
{
	return m_suspensionDamping;
}

void Hinge2JointDesc::setSuspensionStiffness(float suspensionStiffness)
{
	m_suspensionStiffness = suspensionStiffness;
}

float Hinge2JointDesc::getSuspensionStiffness() const
{
	return m_suspensionStiffness;
}

void Hinge2JointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor, AttributePoint());
	s >> Member< Vector4 >(L"axis1", m_axis1, AttributeDirection());
	s >> Member< Vector4 >(L"axis2", m_axis2, AttributeDirection());
	s >> Member< float >(L"lowStop", m_lowStop, AttributeRange(-PI, PI));
	s >> Member< float >(L"highStop", m_highStop, AttributeRange(-PI, PI));
	s >> Member< bool >(L"suspensionEnable", m_suspensionEnable);
	s >> Member< float >(L"suspensionDamping", m_suspensionDamping, AttributeRange(0.0f));
	s >> Member< float >(L"suspensionStiffness", m_suspensionStiffness, AttributeRange(0.0f));
}

	}
}
