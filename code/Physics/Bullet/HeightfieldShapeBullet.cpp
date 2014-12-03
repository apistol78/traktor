#include "Core/Log/Log.h"
#include "Core/Math/Aabb3.h"
#include "Heightfield/Heightfield.h"
#include "Physics/Bullet/Conversion.h"
#include "Physics/Bullet/HeightfieldShapeBullet.h"

namespace traktor
{
	namespace physics
	{
		namespace
		{

const int32_t c_gridMinMaxStep = 8;
const int32_t c_gridMinMaxStepSkip = 4;
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
{
	m_shapeType = TERRAIN_SHAPE_PROXYTYPE;

	// Build grid of min/max heights.
	int32_t size = m_heightfield->getSize();	// Size in N*N heights

	// Allocate min/max 2d grid.
	int32_t gridSize = size / c_gridMinMaxStep;
	m_gridMinMax.reset(new MinMax [gridSize * gridSize]);

	// Calculate min/max 2d grid.
	for (int32_t z = 0; z < gridSize; ++z)
	{
		for (int32_t x = 0; x < gridSize; ++x)
		{
			int32_t offset = x + z * gridSize;

			int32_t hx0 = x * c_gridMinMaxStep;
			int32_t hz0 = z * c_gridMinMaxStep;

			int32_t hx1 = hx0 + c_gridMinMaxStep;
			int32_t hz1 = hz0 + c_gridMinMaxStep;

			m_gridMinMax[offset].mn = std::numeric_limits< float >::max();
			m_gridMinMax[offset].mx = -std::numeric_limits< float >::max();

			for (int32_t hz = hz0; hz < hz1; ++hz)
			{
				for (int32_t hx = hx0; hx < hx1; ++hx)
				{
					float hu = heightfield->getGridHeightNearest(hx, hz);
					float hw = heightfield->unitToWorld(hu);

					m_gridMinMax[offset].mn = std::min(m_gridMinMax[offset].mn, hw);
					m_gridMinMax[offset].mx = std::max(m_gridMinMax[offset].mx, hw);
				}
			}
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

void HeightfieldShapeBullet::processRaycastAllTriangles(btTriangleRaycastCallback *callback, const btVector3 &raySource, const btVector3 &rayTarget)
{
	int32_t x0, z0;
	int32_t x1, z1;
	
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

	int32_t size = m_heightfield->getSize();
	int32_t gridSize = size / c_gridMinMaxStep;

	x0 = clamp(x0, 0, size - 1);
	z0 = clamp(z0, 0, size - 1);

	x1 = clamp(x1, 0, size - 1);
	z1 = clamp(z1, 0, size - 1);

	if (x0 > x1)
		std::swap(x0, x1);
	if (z0 > z1)
		std::swap(z0, z1);

	Vector4 rs = Vector4::loadAligned(raySource.m_floats).xyz1();
	Vector4 rt = Vector4::loadAligned(rayTarget.m_floats).xyz1();

	for (int32_t z = z0; z <= z1; z += c_gridMinMaxStep)
	{
		for (int32_t x = x0; x <= x1; x += c_gridMinMaxStep)
		{
			int32_t gx = x / c_gridMinMaxStep;
			int32_t gz = z / c_gridMinMaxStep;

			int32_t offset = gx + gz * gridSize;

			float wx0, wz0;
			m_heightfield->gridToWorld(
				x,
				z,
				wx0,
				wz0
			);

			float wx1, wz1;
			m_heightfield->gridToWorld(
				x + c_gridMinMaxStep - 1,
				z + c_gridMinMaxStep - 1,
				wx1,
				wz1
			);

			Aabb3 aabb(
				Vector4(wx0, m_gridMinMax[offset].mn, wz0),
				Vector4(wx1, m_gridMinMax[offset].mx, wz1)
			);

			Scalar d;
			if (aabb.intersectSegment(rs, rt, d))
			{
				btVector3 triangles[2][3];

				for (int32_t zz = z; zz < z + c_gridMinMaxStep; zz += c_gridMinMaxStepSkip)
				{
					for (int32_t xx = x; xx < x + c_gridMinMaxStep; xx += c_gridMinMaxStepSkip)
					{
						bool c[] =
						{
							m_heightfield->getGridCut(xx                       , zz                       ),
							m_heightfield->getGridCut(xx + c_gridMinMaxStepSkip, zz                       ),
							m_heightfield->getGridCut(xx + c_gridMinMaxStepSkip, zz + c_gridMinMaxStepSkip),
							m_heightfield->getGridCut(xx                       , zz + c_gridMinMaxStepSkip)
						};

						if (c[0] && c[2])
						{
							float h[] =
							{
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx                       , zz                       )),
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx + c_gridMinMaxStepSkip, zz                       )),
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx + c_gridMinMaxStepSkip, zz + c_gridMinMaxStepSkip)),
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx                       , zz + c_gridMinMaxStepSkip))
							};

							float mnx, mnz;
							m_heightfield->gridToWorld(xx, zz, mnx, mnz);

							float mxx, mxz;
							m_heightfield->gridToWorld(xx + c_gridMinMaxStepSkip, zz + c_gridMinMaxStepSkip, mxx, mxz);

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
					}
				}
			}
		}
	}
}

void HeightfieldShapeBullet::processConvexcastAllTriangles(btTriangleConvexcastCallback *callback, const btVector3 &boxSource, const btVector3 &boxTarget, const btVector3 &boxMin, const btVector3 &boxMax)
{
	int32_t x0, z0;
	int32_t x1, z1;
	
	m_heightfield->worldToGrid(
		boxSource.x(),
		boxSource.z(),
		x0,
		z0
	);

	m_heightfield->worldToGrid(
		boxTarget.x(),
		boxTarget.z(),
		x1,
		z1
	);

	int32_t size = m_heightfield->getSize();
	int32_t gridSize = size / c_gridMinMaxStep;

	x0 = clamp(x0, 0, size - 1);
	z0 = clamp(z0, 0, size - 1);

	x1 = clamp(x1, 0, size - 1);
	z1 = clamp(z1, 0, size - 1);

	if (x0 > x1)
		std::swap(x0, x1);
	if (z0 > z1)
		std::swap(z0, z1);

	Vector4 rs = Vector4::loadAligned(boxSource.m_floats).xyz1();
	Vector4 rt = Vector4::loadAligned(boxTarget.m_floats).xyz1();

	Vector4 bmn = Vector4::loadAligned(boxMin.m_floats).xyz0();
	Vector4 bmx = Vector4::loadAligned(boxMax.m_floats).xyz0();

	for (int32_t z = z0; z <= z1; z += c_gridMinMaxStep)
	{
		for (int32_t x = x0; x <= x1; x += c_gridMinMaxStep)
		{
			int32_t gx = x / c_gridMinMaxStep;
			int32_t gz = z / c_gridMinMaxStep;

			int32_t offset = gx + gz * gridSize;

			float wx0, wz0;
			m_heightfield->gridToWorld(
				x,
				z,
				wx0,
				wz0
			);

			float wx1, wz1;
			m_heightfield->gridToWorld(
				x + c_gridMinMaxStep - 1,
				z + c_gridMinMaxStep - 1,
				wx1,
				wz1
			);

			Aabb3 aabb(
				Vector4(wx0, m_gridMinMax[offset].mn, wz0) + bmn,
				Vector4(wx1, m_gridMinMax[offset].mx, wz1) + bmx
			);

			Scalar d;
			if (aabb.intersectSegment(rs, rt, d))
			{
				btVector3 triangles[2][3];

				for (int32_t zz = z; zz < z + c_gridMinMaxStep; zz += c_gridMinMaxStepSkip)
				{
					for (int32_t xx = x; xx < x + c_gridMinMaxStep; xx += c_gridMinMaxStepSkip)
					{
						bool c[] =
						{
							m_heightfield->getGridCut(xx                       , zz                       ),
							m_heightfield->getGridCut(xx + c_gridMinMaxStepSkip, zz                       ),
							m_heightfield->getGridCut(xx + c_gridMinMaxStepSkip, zz + c_gridMinMaxStepSkip),
							m_heightfield->getGridCut(xx                       , zz + c_gridMinMaxStepSkip)
						};

						if (c[0] && c[2])
						{
							float h[] =
							{
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx                       , zz                       )),
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx + c_gridMinMaxStepSkip, zz                       )),
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx + c_gridMinMaxStepSkip, zz + c_gridMinMaxStepSkip)),
								m_heightfield->unitToWorld(m_heightfield->getGridHeightNearest(xx                       , zz + c_gridMinMaxStepSkip))
							};

							float mnx, mnz;
							m_heightfield->gridToWorld(xx, zz, mnx, mnz);

							float mxx, mxz;
							m_heightfield->gridToWorld(xx + c_gridMinMaxStepSkip, zz + c_gridMinMaxStepSkip, mxx, mxz);

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
					}
				}
			}
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
