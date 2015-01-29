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
const int32_t c_raycastStepSize = 4;

inline float quantizeMin(float v)
{
	return floorf(v);
}

inline float quantizeMax(float v)
{
	return ceilf(v);
}

template < typename Visitor >
void line_dda(float x0, float y0, float x1, float y1, Visitor& visitor)
{
	float dx = x1 - x0;
	float dy = y1 - y0;

	float dln = std::sqrt(dx * dx + dy * dy);
	if (dln > FUZZY_EPSILON)
	{
		dx /= dln;
		dy /= dln;
	}

	float x = std::floor(x0);
	float y = std::floor(y0);

	float stepx, stepy;
	float cbx, cby;

	if (dx > 0.0f)
	{
		stepx = 1.0f;
		cbx = x + 1.0f;
	}
	else
	{
		stepx = -1.0f;
		cbx = x;
	}

	if (dy > 0.0f)
	{
		stepy = 1.0f;
		cby = y + 1.0f;
	}
	else
	{
		stepy = -1.0f;
		cby = y;
	}

	float tmaxx, tmaxy;
	float tdeltax, tdeltay;
	float rxr, ryr;

	if (std::abs(dx) > FUZZY_EPSILON)
	{
		rxr = 1.0f / dx;
		tmaxx = (cbx - x0) * rxr;
		tdeltax = stepx * rxr;
	}
	else
		tmaxx = std::numeric_limits< float >::max();

	if (std::abs(dy) > FUZZY_EPSILON)
	{
		ryr = 1.0f / dy;
		tmaxy = (cby - y0) * ryr;
		tdeltay = stepy * ryr;
	}
	else
		tmaxy = std::numeric_limits< float >::max();

	int32_t ix1 = int32_t(x1);
	int32_t iy1 = int32_t(y1);

	for (int32_t i = 0; i < 10000; ++i)
	{
		int32_t ix = int32_t(x);
		int32_t iy = int32_t(y);

		visitor(ix, iy);

		if (ix == ix1 && iy == iy1)
			break;

		if (tmaxx < tmaxy)
		{
			x += stepx;
			tmaxx += tdeltax;
		}
		else
		{
			y += stepy;
			tmaxy += tdeltay;
		}
	}
}

struct ProcessRaycastAllTrianglesVisitor
{
	hf::Heightfield* heightfield;
	btTriangleRaycastCallback* callback;
	btVector3 triangles[3];

	void operator () (int32_t x, int32_t z)
	{
		int32_t xx = x * c_raycastStepSize;
		int32_t zz = z * c_raycastStepSize;

		bool c[] =
		{
			heightfield->getGridCut(xx, zz),
			heightfield->getGridCut(xx + c_raycastStepSize, zz),
			heightfield->getGridCut(xx + c_raycastStepSize, zz + c_raycastStepSize),
			heightfield->getGridCut(xx, zz + c_raycastStepSize)
		};

		if (c[0] && c[2])
		{
			float h[] =
			{
				heightfield->unitToWorld(heightfield->getGridHeightNearest(xx, zz)),
				heightfield->unitToWorld(heightfield->getGridHeightNearest(xx + c_raycastStepSize, zz)),
				heightfield->unitToWorld(heightfield->getGridHeightNearest(xx + c_raycastStepSize, zz + c_raycastStepSize)),
				heightfield->unitToWorld(heightfield->getGridHeightNearest(xx, zz + c_raycastStepSize))
			};

			float mnx, mnz;
			heightfield->gridToWorld(xx, zz, mnx, mnz);

			float mxx, mxz;
			heightfield->gridToWorld(xx + c_raycastStepSize, zz + c_raycastStepSize, mxx, mxz);

			if (c[3])
			{
				triangles[0] = btVector3(mnx, h[0], mnz);
				triangles[1] = btVector3(mnx, h[3], mxz);
				triangles[2] = btVector3(mxx, h[2], mxz);
				callback->processTriangle(triangles, 0, 0);
			}
			if (c[1])
			{
				triangles[0] = btVector3(mxx, h[2], mxz);
				triangles[1] = btVector3(mxx, h[1], mnz);
				triangles[2] = btVector3(mnx, h[0], mnz);
				callback->processTriangle(triangles, 0, 1);
			}
		}
	}
};

struct ProcessRaycastAllTrianglesNoCutsVisitor
{
	hf::Heightfield* heightfield;
	btTriangleRaycastCallback* callback;
	btVector3 triangles[3];

	void operator () (int32_t x, int32_t z)
	{
		int32_t xx = x * c_raycastStepSize;
		int32_t zz = z * c_raycastStepSize;

		float h[] =
		{
			heightfield->unitToWorld(heightfield->getGridHeightNearest(xx, zz)),
			heightfield->unitToWorld(heightfield->getGridHeightNearest(xx + c_raycastStepSize, zz)),
			heightfield->unitToWorld(heightfield->getGridHeightNearest(xx + c_raycastStepSize, zz + c_raycastStepSize)),
			heightfield->unitToWorld(heightfield->getGridHeightNearest(xx, zz + c_raycastStepSize))
		};

		float mnx, mnz;
		heightfield->gridToWorld(xx, zz, mnx, mnz);

		float mxx, mxz;
		heightfield->gridToWorld(xx + c_raycastStepSize, zz + c_raycastStepSize, mxx, mxz);

		triangles[0] = btVector3(mnx, h[0], mnz);
		triangles[1] = btVector3(mnx, h[3], mxz);
		triangles[2] = btVector3(mxx, h[2], mxz);
		callback->processTriangle(triangles, 0, 0);

		triangles[0] = btVector3(mxx, h[2], mxz);
		triangles[1] = btVector3(mxx, h[1], mnz);
		triangles[2] = btVector3(mnx, h[0], mnz);
		callback->processTriangle(triangles, 0, 1);
	}
};

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
	for (uint32_t i = 0; i < m_heightfield->getSize() * m_heightfield->getSize() / 8; ++i)
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

#if !defined(_PS3)

void HeightfieldShapeBullet::processRaycastAllTriangles(btTriangleRaycastCallback *callback, const btVector3 &raySource, const btVector3 &rayTarget)
{
	float x0, z0;
	float x1, z1;
	
	m_heightfield->worldToGrid(
		raySource.x(),
		raySource.z(),
		x0,
		z0
	);

	m_heightfield->worldToGrid(
		rayTarget.x(),
		rayTarget.z(),
		x1,
		z1
	);

	x0 /= c_raycastStepSize;
	z0 /= c_raycastStepSize;
	x1 /= c_raycastStepSize;
	z1 /= c_raycastStepSize;

	if (m_haveCuts)
	{
		ProcessRaycastAllTrianglesVisitor prt;
		prt.heightfield = m_heightfield;
		prt.callback = callback;
		line_dda(x0, z0, x1, z1, prt);
	}
	else
	{
		ProcessRaycastAllTrianglesNoCutsVisitor prt;
		prt.heightfield = m_heightfield;
		prt.callback = callback;
		line_dda(x0, z0, x1, z1, prt);
	}
}

#endif

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
