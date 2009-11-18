#include "Physics/Hinge2JointDesc.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Math/Const.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.Hinge2JointDesc", Hinge2JointDesc, JointDesc)

Hinge2JointDesc::Hinge2JointDesc()
:	m_anchor(0.0f, 0.0f, 0.0f, 1.0f)
,	m_axis1(0.0f, 1.0f, 0.0f, 0.0f)
,	m_axis2(1.0f, 0.0f, 0.0f, 0.0f)
,	m_lowStop(0.0f)
,	m_highStop(0.0f)
,	m_suspensionErp(0.4f)
,	m_suspensionCfm(0.8f)
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

void Hinge2JointDesc::setSuspensionErp(float suspensionErp)
{
	m_suspensionErp = suspensionErp;
}

float Hinge2JointDesc::getSuspensionErp() const
{
	return m_suspensionErp;
}

void Hinge2JointDesc::setSuspensionCfm(float suspensionCfm)
{
	m_suspensionCfm = suspensionCfm;
}

float Hinge2JointDesc::getSuspensionCfm() const
{
	return m_suspensionCfm;
}

bool Hinge2JointDesc::serialize(ISerializer& s)
{
	s >> Member< Vector4 >(L"anchor", m_anchor);
	s >> Member< Vector4 >(L"axis1", m_axis1);
	s >> Member< Vector4 >(L"axis2", m_axis2);
	s >> Member< float >(L"lowStop", m_lowStop, -PI, PI);
	s >> Member< float >(L"highStop", m_highStop, -PI, PI);
	s >> Member< float >(L"suspensionErp", m_suspensionErp, 0.0f, 1.0f);
	s >> Member< float >(L"suspensionCfm", m_suspensionCfm, 0.0f, 1.0f);
	return true;
}

	}
}
