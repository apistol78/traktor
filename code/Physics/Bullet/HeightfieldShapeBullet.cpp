#include "Physics/Bullet/HeightfieldShapeBullet.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Heightfield.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

inline float quantizeMin(float v)
{
	return floorf(v - 1.0f);
}

inline float quantizeMax(float v)
{
	return ceilf(v + 1.0f);
}

		}

HeightfieldShapeBullet::HeightfieldShapeBullet(const resource::Proxy< Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_localScaling(1.0f, 1.0f, 1.0f)
{
}

HeightfieldShapeBullet::~HeightfieldShapeBullet()
{
}

int	HeightfieldShapeBullet::getShapeType() const
{
	return TERRAIN_SHAPE_PROXYTYPE;
}

void HeightfieldShapeBullet::setLocalScaling(const btVector3& scaling)
{
}

const btVector3& HeightfieldShapeBullet::getLocalScaling() const
{
	return m_localScaling;
}

void HeightfieldShapeBullet::getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const
{
	if (!m_heightfield.validate())
		return;

	const Vector4& worldExtent = m_heightfield->getWorldExtent();

	btVector3 localAabbMin = toBtVector3(-worldExtent * Scalar(0.5f));
	btVector3 localAabbMax = toBtVector3(worldExtent * Scalar(0.5f));

	btVector3 halfExtents = (localAabbMax - localAabbMin) * btScalar(0.5);

	btMatrix3x3 basisAbs = t.getBasis().absolute();  
	btVector3 center = t.getOrigin();

	btVector3 extent = btVector3(
		basisAbs[0].dot(halfExtents) + getMargin(),
		basisAbs[1].dot(halfExtents) + getMargin(),
		basisAbs[2].dot(halfExtents) + getMargin()
	);

	aabbMin = center - extent;
	aabbMax = center + extent;
}

void HeightfieldShapeBullet::processAllTriangles(btTriangleCallback* callback, const btVector3& aabbMin, const btVector3& aabbMax) const
{
	if (!m_heightfield.validate())
		return;

	float mnx = quantizeMin(aabbMin.x());
	float mxx = quantizeMax(aabbMax.x());
	float mnz = quantizeMin(aabbMin.z());
	float mxz = quantizeMax(aabbMax.z());

	for (int u = 0; u < 4; ++u)
	{
		float mnx2 = mnx + (mxx - mnx) * u / 4.0f;
		float mxx2 = mnx + (mxx - mnx) * (u + 1.0f) / 4.0f;

		for (int v = 0; v < 4; ++v)
		{
			float mnz2 = mnz + (mxz - mnz) * v / 4.0f;
			float mxz2 = mnz + (mxz - mnz) * (v + 1.0f) / 4.0f;

			float h[] =
			{
				m_heightfield->getSampleBilinear(mnx2, mnz2),
				m_heightfield->getSampleBilinear(mxx2, mnz2),
				m_heightfield->getSampleBilinear(mxx2, mxz2),
				m_heightfield->getSampleBilinear(mnx2, mxz2)
			};

			btVector3 triangles[][3] =
			{
				{
					btVector3(mnx2, h[0], mnz2),
					btVector3(mnx2, h[3], mxz2),
					btVector3(mxx2, h[2], mxz2)
				},
				{
					btVector3(mxx2, h[2], mxz2),
					btVector3(mxx2, h[1], mnz2),
					btVector3(mnx2, h[0], mnz2)
				}
			};

			callback->processTriangle(triangles[0], 0, 0);
			callback->processTriangle(triangles[1], 0, 1);
		}
	}
}

void HeightfieldShapeBullet::calculateLocalInertia(btScalar mass, btVector3& inertia) const
{
	inertia.setValue(btScalar(0.), btScalar(0.), btScalar(0.));
}

const char*	HeightfieldShapeBullet::getName() const
{
	return "HEIGHTFIELD";
}

	}
}
