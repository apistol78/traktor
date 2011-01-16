#include <limits>
#include "Core/Math/Aabb2.h"
#include "Core/Math/Line2.h"
#include "Core/Math/Winding2.h"
#include "Render/PrimitiveRenderer.h"
#include "World/WorldRenderSettings.h"
#include "World/SMProj/TSMProj.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

Vector4 v2tov4(const Vector2& v)
{
	return Vector4(v.x, v.y, 0.0f, 0.0f);
}

Vector4 v2top4(const Vector2& v)
{
	return Vector4(v.x, v.y, 0.0f, 1.0f);
}

		}

void calculateTSMProj(
	const WorldRenderSettings& settings,
	const Matrix44& viewInverse,
	const Vector4& lightPosition,
	const Vector4& lightDirection,
	const Frustum& viewFrustum,
	Matrix44& outLightView,
	Matrix44& outLightProjection,
	Matrix44& outLightSquareProjection,
	Frustum& outShadowFrustum,
	render::PrimitiveRenderer* primitiveRenderer
)
{
	Vector4 viewDirection = viewInverse.axisZ();

	// Calculate light axises.
	Vector4 lightAxisX, lightAxisY, lightAxisZ;

	lightAxisZ = -lightDirection.normalized();
	lightAxisX = cross(lightAxisZ, -viewDirection).normalized();
	lightAxisY = -cross(lightAxisX, lightAxisZ).normalized();

	// Adjust view frustum to shadowing far z.
	Frustum shadowViewFrustum = viewFrustum;
	shadowViewFrustum.setFarZ(Scalar(settings.shadowFarZ));

	// Calculate X/Y projection of view frustum in light space.
	AlignedVector< Vector2 > lightFrustumProj;
	Aabb3 viewFrustumBox;
	Aabb2 lightFrustumProjBox;
	for (int i = 0; i < 8; ++i)
	{
		Vector4 worldCorner = viewInverse * shadowViewFrustum.corners[i];
		Vector4 lightCorner(
			dot3(lightAxisX, worldCorner),
			dot3(lightAxisY, worldCorner),
			dot3(lightAxisZ, worldCorner),
			1.0f
		);
		viewFrustumBox.contain(lightCorner);
		lightFrustumProj.push_back(Vector2(
			lightCorner.x(),
			lightCorner.y()
		));
		lightFrustumProjBox.contain(lightFrustumProj.back());
	}

	if (primitiveRenderer)
	{
		const int* edges = Aabb3::getEdges();
		for (int i = 0; i < 12; ++i)
		{
			primitiveRenderer->drawLine(
				v2top4(lightFrustumProj[edges[i * 2 + 0]] - lightFrustumProjBox.getCenter()),
				v2top4(lightFrustumProj[edges[i * 2 + 1]] - lightFrustumProjBox.getCenter()),
				Color4ub(255, 255, 0, 255)
			);
		}
	}

	// Projection is already orientated properly; thus we can
	// quickly approximate best fitting, symmetrical, trapezoid.
	float ey = lightFrustumProjBox.getExtent().y;
	float nw = 0.0f, fw = 0.0f;
	for (int i = 0; i < 8; ++i)
	{
		Vector2 p = lightFrustumProj[i] - lightFrustumProjBox.getCenter();
		if (i < 4)
			nw = std::max(nw, abs(p.x));
		else
		{
			Vector2 p0(p.x < 0.0f ? -nw : nw, -ey);
			Vector2 p1(p.x, p.y);
			Vector2 d = (p1 - p0).normalized();
			float w = p.x + d.x * (ey - p.y);
			fw = std::max(fw, w);
		}
	}
	Line2 tz[2] =
	{
		Line2(Vector2(-nw, 0.0f), Vector2(nw, 0.0f)),
		Line2(Vector2(-fw, ey * 2.0f), Vector2(fw, ey * 2.0f))
	};

	if (primitiveRenderer)
	{
		primitiveRenderer->drawLine(v2top4(tz[0].p[0]), v2top4(tz[0].p[1]), Color4ub(0, 0, 255, 255));
		primitiveRenderer->drawLine(v2top4(tz[1].p[0]), v2top4(tz[1].p[1]), Color4ub(0, 0, 255, 255));
		primitiveRenderer->drawLine(v2top4(tz[0].p[0]), v2top4(tz[1].p[0]), Color4ub(0, 0, 255, 255));
		primitiveRenderer->drawLine(v2top4(tz[0].p[1]), v2top4(tz[1].p[1]), Color4ub(0, 0, 255, 255));
	}

	Vector4 u, v, i;

	// Calculate intersection of trapezoid edges.
	Ray2 r1(tz[1].p[0], (tz[0].p[0] - tz[1].p[0]).normalized());
	Ray2 r2(tz[1].p[1], (tz[0].p[1] - tz[1].p[1]).normalized());
	float r, k;
	if (r1.intersect(r2, r, k))
		i = v2top4(r1 * r);
	else
		i.set(0.0f, 0.0f, 0.0f, 0.0f);

	Matrix44 T2(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -i.y(),
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	u = T2 * v2top4(tz[0].p[1]);
	Matrix44 S1(
		1.0f / u.x(), 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f / u.y(), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 N(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f
	);

	Matrix44 N_S1_T2 = N * S1 * T2;
	u = N_S1_T2 * v2top4(tz[1].p[0]);
	v = N_S1_T2 * v2top4(tz[0].p[1]);
	Matrix44 T3(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, -(u.y() / u.w() + v.y() / v.w()) / 2.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 T3_N_S1_T2 = T3 * N_S1_T2;
	u = T3_N_S1_T2 * v2top4(tz[1].p[0]);
	Matrix44 S2(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, u.w() / u.y(), 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	Matrix44 NT = S2 * T3_N_S1_T2;

	if (primitiveRenderer)
	{
		primitiveRenderer->pushWorld(NT);
		primitiveRenderer->drawLine(v2top4(tz[0].p[0]), v2top4(tz[0].p[1]), 3, Color4ub(255, 0, 0, 255));
		primitiveRenderer->drawLine(v2top4(tz[1].p[0]), v2top4(tz[1].p[1]), 3, Color4ub(255, 0, 255, 255));
		primitiveRenderer->popWorld();
	}

	// Update light view matrix with bounding box centered.
	Vector4 center = viewFrustumBox.getCenter();
	Vector4 extent = viewFrustumBox.getExtent() * Vector4(2.0f, 2.0f, 1.0f, 0.0f);

	float cy = center.y();
	float nz = shadowViewFrustum.getNearZ();

	// Calculate light view and projection matrices.
	Vector4 worldCenter = viewInverse * Vector4(0.0f, 0.0f, shadowViewFrustum.getNearZ(), 1.0f);
	Scalar lightDistance = Scalar(settings.depthRange);

	outLightView = Matrix44(
		lightAxisX,
		lightAxisY,
		lightAxisZ,
		worldCenter - lightAxisZ * lightDistance
	);

	outLightView = outLightView.inverseOrtho();

	outLightProjection = orthoLh(
		2.0f,
		2.0f,
		0.0f,
		lightDistance + extent.z()
	);

	outLightSquareProjection = NT;

	outShadowFrustum.buildOrtho(
		extent.x(),
		extent.y(),
		0.0f,
		lightDistance + extent.z()
	);
}

	}
}
