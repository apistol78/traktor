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
}

void StaticBodyOde::setTransform(const Matrix44& transform)
{
	Vector4 p = transform.translation();
	dGeomSetPosition(m_geomId, p.x(), p.y(), p.z());

	dMatrix3 rotation =
	{
		transform(0, 0), transform(1, 0), transform(2, 0), 0.0f,
		transform(0, 1), transform(1, 1), transform(2, 1), 0.0f,
		transform(0, 2), transform(1, 2), transform(2, 2), 0.0f
	};
	dGeomSetRotation(m_geomId, rotation);
}

Matrix44 StaticBodyOde::getTransform() const
{
	const dReal* p = dGeomGetPosition(m_geomId);
	const dReal* r = dGeomGetRotation(m_geomId);
	return Matrix44(
		r[0], r[4], r[8] , 0.0f,
		r[1], r[5], r[9] , 0.0f,
		r[2], r[6], r[10], 0.0f,
		p[0], p[1], p[2] , 1.0f
	);
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
