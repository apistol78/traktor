/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <limits>
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/Align.h"
#include "Heightfield/Heightfield.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/HeightfieldShapeBullet.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const int32_t c_maximumStep = 32;

inline float quantizeMin(float v)
{
	return floorf(v);
}

inline float quantizeMax(float v)
{
	return ceilf(v);
}

		}

HeightfieldShapeBullet::HeightfieldShapeBullet(const resource::Proxy< hf::Heightfield >& heightfield)
:	m_heightfield(heightfield)
,	m_localScaling(1.0f, 1.0f, 1.0f)
,	m_haveCuts(false)
{
	m_shapeType = TERRAIN_SHAPE_PROXYTYPE;

	// Check if we need to account for cuts in the heightfield;
	// heightfields with no cuts are slightly faster to check.
	const uint8_t* cuts = m_heightfield->getCuts();
	for (int32_t i = 0; i < m_heightfield->getSize() * m_heightfield->getSize() / 8; ++i)
	{
		if (cuts[i] != 0xff)
		{
			m_haveCuts = true;
			break;
		}
	}
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

	float cxf = float(cx);
	float czf = float(cz);

	mnx = float(imnx);
	mxx = mnx + cxf;

	btVector3 triangles[2][3];
	
	for (int32_t u = imnx; u < imxx; u += cx)
	{
		mnz = float(imnz);
		mxz = mnz + czf;

		bool cp0 = m_heightfield->getWorldCut(mnx, mnz);
		bool cp1 = m_heightfield->getWorldCut(mxx, mnz);
		float hp0 = m_heightfield->getWorldHeight(mnx, mnz);
		float hp1 = m_heightfield->getWorldHeight(mxx, mnz);

		for (int32_t v = imnz; v < imxz; v += cz)
		{
			bool c[] =
			{
				cp0,
				cp1,
				m_heightfield->getWorldCut(mxx, mxz),
				m_heightfield->getWorldCut(mnx, mxz)
			};

			cp0 = c[3];
			cp1 = c[2];

			float h[] =
			{
				hp0,
				hp1,
				m_heightfield->getWorldHeight(mxx, mxz),
				m_heightfield->getWorldHeight(mnx, mxz)
			};

			hp0 = h[3];
			hp1 = h[2];

			if (c[0] && c[2])
			{
				triangles[0][0] = btVector3(mnx, h[0], mnz);
				triangles[0][1] = btVector3(mnx, h[3], mxz);
				triangles[0][2] = btVector3(mxx, h[2], mxz);

				triangles[1][0] = btVector3(mxx, h[2], mxz);
				triangles[1][1] = btVector3(mxx, h[1], mnz);
				triangles[1][2] = btVector3(mnx, h[0], mnz);

				if (c[3])
					callback->processTriangle(triangles[0], 0, 0);
				if (c[1])
					callback->processTriangle(triangles[1], 0, 1);
			}

			mnz += czf;
			mxz += czf;
		}

		mnx += cxf;
		mxx += cxf;
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
