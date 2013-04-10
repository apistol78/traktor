#include "Heightfield/Heightfield.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/HeightfieldShapeBullet.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const int32_t c_maximumStep = 16;

inline float quantizeMin(float v)
{
	return floorf(v - 1.0f);
}

inline float quantizeMax(float v)
{
	return ceilf(v + 1.0f);
}

		}

HeightfieldShapeBullet::HeightfieldShapeBullet(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_localScaling(1.0f, 1.0f, 1.0f)
{
	m_shapeType = TERRAIN_SHAPE_PROXYTYPE;
}

HeightfieldShapeBullet::~HeightfieldShapeBullet()
{
}

void HeightfieldShapeBullet::setLocalScaling(const btVector3& scaling)
{
	m_localScaling = scaling;
}

const btVector3& HeightfieldShapeBullet::getLocalScaling() const
{
	return m_localScaling;
}

void HeightfieldShapeBullet::getAabb(const btTransform& t, btVector3& aabbMin, btVector3& aabbMax) const
{
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
	float mnx = quantizeMin(aabbMin.x());
	float mxx = quantizeMax(aabbMax.x());
	float mnz = quantizeMin(aabbMin.z());
	float mxz = quantizeMax(aabbMax.z());

	int32_t imnx = int32_t(mnx);
	int32_t imxx = int32_t(mxx);
	int32_t imnz = int32_t(mnz);
	int32_t imxz = int32_t(mxz);

	if (imnx >= imxx || imnz >= imxz)
		return;

	int32_t cx = ((imxx - imnx) >> 1) | 1;
	int32_t cz = ((imxz - imnz) >> 1) | 1;

	cx = min(cx, c_maximumStep);
	cz = min(cz, c_maximumStep);

	for (int32_t u = imnx; u < imxx; u += cx)
	{
		float mnx2 = float(u);
		float mxx2 = float(u + cx);

		for (int32_t v = imnz; v < imxz; v += cz)
		{
			float mnz2 = float(v);
			float mxz2 = float(v + cz);

			float h[] =
			{
				m_heightfield->getWorldHeight(mnx2, mnz2),
				m_heightfield->getWorldHeight(mxx2, mnz2),
				m_heightfield->getWorldHeight(mxx2, mxz2),
				m_heightfield->getWorldHeight(mnx2, mxz2)
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
