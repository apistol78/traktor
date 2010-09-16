#include "Physics/Ode/StaticBodyOde.h"
#include "Physics/Ode/Types.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.StaticBodyOde", StaticBodyOde, StaticBody)

StaticBodyOde::StaticBodyOde(DestroyCallback* callback, dGeomID geomId)
:	m_callback(callback)
,	m_geomId(geomId)
{
	dGeomSetData(m_geomId, (void*)this);
}

StaticBodyOde::~StaticBodyOde()
{
	destroy();
}

void StaticBodyOde::destroy()
{
	if (m_callback)
	{
		m_callback->bodyDestroyed(this);
		m_callback = 0;
	}
	if (m_geomId)
	{
		dGeomDestroy(m_geomId);
		m_geomId = 0;
	}
	StaticBody::destroy();
}

void StaticBodyOde::setTransform(const Transform& transform)
{
	T_ASSERT (m_geomId != 0);

	const Vector4& p = transform.translation();
	dGeomSetPosition(m_geomId, p.x(), p.y(), p.z());

	Matrix44 tm = transform.toMatrix44();
	dMatrix3 rotation =
	{
		tm(0, 0), tm(0, 1), tm(0, 2), 0.0f,
		tm(1, 0), tm(1, 1), tm(1, 2), 0.0f,
		tm(2, 0), tm(2, 1), tm(2, 2), 0.0f
	};
	dGeomSetRotation(m_geomId, rotation);
}

Transform StaticBodyOde::getTransform() const
{
	T_ASSERT (m_geomId != 0);

	const dReal* p = dGeomGetPosition(m_geomId);
	const dReal* r = dGeomGetRotation(m_geomId);

	return Transform(Matrix44(
		r[0], r[1], r[2] , p[0],
		r[4], r[5], r[6] , p[1],
		r[8], r[9], r[10], p[2],
		0.0f, 0.0f,  0.0f, 1.0f
	));
}

void StaticBodyOde::setActive(bool active)
{
}

bool StaticBodyOde::isActive() const
{
	return true;
}

void StaticBodyOde::setEnable(bool enable)
{
}

bool StaticBodyOde::isEnable() const
{
	return true;
}

const dGeomID StaticBodyOde::getGeomId() const
{
	return m_geomId;
}

	}
}
