#include <cstring>
#include <limits>
#include "Core/Math/Float.h"
#include "Core/Math/Log2.h"
#include "Core/Math/MathUtils.h"
#include "Core/Math/Triangle.h"
#include "World/WorldRenderView.h"
#include "World/SwHiZ/OccluderMesh.h"
#include "World/SwHiZ/WorldCullingSwRaster.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

const int32_t c_width = 256;
const int32_t c_height = 144;
const uint32_t c_maxVertices = 4096;
const Scalar c_one(1.0f);
const Scalar c_half(0.5f);
const Scalar c_zero(0.0f);

struct FragmentVisitor
{
	uint16_t* depth;
	float iw0;
	float iw1;
	float iw2;

	void operator () (int32_t x, int32_t y, float alpha, float beta, float gamma)
	{
		float iw = iw0 * alpha + iw1 * beta + iw2 * gamma;
		uint16_t df = uint16_t(iw * 65535.0f);
		uint16_t& dc = depth[x + y * c_width];
		if (df > dc)
			dc = df;
	}
};

struct T_MATH_ALIGN16 Triangle
{
	Vector4 v[3];
};

Scalar getComponentX(const Vector4& v) { return v.x(); }
Scalar getComponentY(const Vector4& v) { return v.y(); }
Scalar getComponentZ(const Vector4& v) { return v.z(); }

bool isOutsideP(const Scalar& x, const Scalar& w) { return x > w; }
bool isOutsideN(const Scalar& x, const Scalar& w) { return x < -w; }

bool clipP(const Scalar& x0, const Scalar& w0, const Scalar& x1, const Scalar& w1, Scalar& t)
{
	Scalar denom = w0 - w1 - x0 + x1;
	if (denom == 0.0f)
		return false;

	t = (w0 - x0) / denom;
	if (t >= 0.0f && t <= 1.0f)
		return true;
	else
		return false;
}

bool clipN(const Scalar& x0, const Scalar& w0, const Scalar& x1, const Scalar& w1, Scalar& t)
{
	Scalar denom = w0 - w1 + x0 - x1;
	if (denom == 0.0f)
		return false;

	t = (w0 + x0) / denom;
	if (t >= 0.0f && t <= 1.0f)
		return true;
	else
		return false;
}

typedef Scalar (*get_component_t)(const Vector4&);
typedef bool (*is_outside_t)(const Scalar& x, const Scalar& w);
typedef bool (*clip_t)(const Scalar& x0, const Scalar& w0, const Scalar& x1, const Scalar& w1, Scalar& t);

const get_component_t c_get[] = { &getComponentX, &getComponentX, &getComponentY, &getComponentY, &getComponentZ, &getComponentZ };
const is_outside_t c_outside[] = { &isOutsideP, &isOutsideN, &isOutsideP, &isOutsideN, &isOutsideP, &isOutsideN };
const clip_t c_clip[] = { &clipP, &clipN, &clipP, &clipN, &clipP, &clipN };

void clipTriangles(StaticVector< Triangle, 8 >& triangles)
{
	StaticVector< Triangle, 8 > remaining;
	Scalar ta, tb;
	Triangle Tc;

	for (int32_t plane = 0; plane < 6; ++plane)
	{
		for (uint32_t i = 0; i < triangles.size(); ++i)
		{
			const Triangle& T = triangles[i];

			Scalar c0 = c_get[plane](T.v[0]);
			Scalar c1 = c_get[plane](T.v[1]);
			Scalar c2 = c_get[plane](T.v[2]);

			Scalar w0 = T.v[0].w();
			Scalar w1 = T.v[1].w();
			Scalar w2 = T.v[2].w();

			bool outside0 = c_outside[plane](c0, w0);
			bool outside1 = c_outside[plane](c1, w1);
			bool outside2 = c_outside[plane](c2, w2);

			const clip_t& clip = c_clip[plane];

			if (outside0 && outside1 && outside2)
				// Whole triangle clipped
				;
			else if (!outside0 && outside1 && outside2)
			{
				clip(c1, w1, c0, w0, ta);
				clip(c2, w2, c0, w0, tb);

				Tc.v[0] = T.v[0];
				Tc.v[1] = lerp(T.v[1], T.v[0], Scalar(ta));
				Tc.v[2] = lerp(T.v[2], T.v[0], Scalar(tb));

				remaining.push_back(Tc);
			}
			else if (outside0 && !outside1 && outside2)
			{
				clip(c2, w2, c1, w1, ta);
				clip(c0, w0, c1, w1, tb);

				Tc.v[0] = T.v[1];
				Tc.v[1] = lerp(T.v[2], T.v[1], Scalar(ta));
				Tc.v[2] = lerp(T.v[0], T.v[1], Scalar(tb));

				remaining.push_back(Tc);
			}
			else if (outside0 && outside1 && !outside2)
			{
				clip(c0, w0, c2, w2, ta);
				clip(c1, w1, c2, w2, tb);

				Tc.v[0] = T.v[2];
				Tc.v[1] = lerp(T.v[0], T.v[2], Scalar(ta));
				Tc.v[2] = lerp(T.v[1], T.v[2], Scalar(tb));

				remaining.push_back(Tc);
			}
			else if (!outside0 && !outside1 && outside2)
			{
				clip(c2, w2, c1, w1, ta);
				clip(c2, w2, c0, w0, tb);

				Vector4 v0 = lerp(T.v[2], T.v[1], Scalar(ta));
				Vector4 v1 = lerp(T.v[2], T.v[0], Scalar(tb));

				Tc.v[0] = T.v[0];
				Tc.v[1] = T.v[1];
				Tc.v[2] = v0;

				remaining.push_back(Tc);

				Tc.v[0] = T.v[0];
				Tc.v[1] = v0;
				Tc.v[2] = v1;

				remaining.push_back(Tc);
			}
			else if (!outside0 && outside1 && !outside2)
			{
				clip(c1, w1, c0, w0, ta);
				clip(c1, w1, c2, w2, tb);

				Vector4 v0 = lerp(T.v[1], T.v[0], Scalar(ta));
				Vector4 v1 = lerp(T.v[1], T.v[2], Scalar(tb));

				Tc.v[0] = T.v[2];
				Tc.v[1] = T.v[0];
				Tc.v[2] = v0;

				remaining.push_back(Tc);

				Tc.v[0] = T.v[2];
				Tc.v[1] = v0;
				Tc.v[2] = v1;

				remaining.push_back(Tc);
			}
			else if (outside0 && !outside1 && !outside2)
			{
				clip(c0, w0, c1, w1, ta);
				clip(c0, w0, c2, w2, tb);

				Vector4 v0 = lerp(T.v[0], T.v[1], Scalar(ta));
				Vector4 v1 = lerp(T.v[0], T.v[2], Scalar(tb));

				Tc.v[0] = T.v[1];
				Tc.v[1] = T.v[2];
				Tc.v[2] = v1;

				remaining.push_back(Tc);

				Tc.v[0] = T.v[1];
				Tc.v[1] = v1;
				Tc.v[2] = v0;

				remaining.push_back(Tc);
			}
			else if (!outside0 && !outside1 && !outside2)
			{
				// Nothing clipped.
				remaining.push_back(T);
			}
		}

		// Swap triangles.
		triangles = remaining;
		remaining.clear();

		// Skip further clipping if no triangles left.
		if (triangles.empty())
			break;
	}
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.WorldCullingSwRaster", WorldCullingSwRaster, IWorldCulling)

WorldCullingSwRaster::WorldCullingSwRaster()
{
	int32_t mipCount = log2(std::max(c_width, c_height)) + 1;
	T_ASSERT (mipCount <= sizeof_array(m_depth));

	for (int32_t i = 0; i < mipCount; ++i)
	{
		int32_t mipWidth = std::max(c_width >> i, 1);
		int32_t mipHeight = std::max(c_height >> i, 1);
		m_depth[i].reset(new uint16_t [mipWidth * mipHeight]);
	}

	m_clipVertices.resize(c_maxVertices);
}

void WorldCullingSwRaster::beginPrecull(const WorldRenderView& worldRenderView)
{
	// Clear depth buffer with far z.
	std::memset(
		m_depth[0].ptr(),
		0,
		c_width * c_height * sizeof(uint16_t)
	);

	// Save projection.
	m_projection = worldRenderView.getProjection();
	m_view = worldRenderView.getView();
}

void WorldCullingSwRaster::endPrecull()
{
	// Create mip chain.
	int32_t mipCount = log2(std::max(c_width, c_height)) + 1;
	T_ASSERT (mipCount <= sizeof_array(m_depth));

	for (int32_t i = 1; i < mipCount; ++i)
	{
		int32_t fromMipWidth = std::max(c_width >> (i - 1), 1);
		int32_t targetMipWidth = std::max(c_width >> i, 1);
		int32_t targetMipHeight = std::max(c_height >> i, 1);

		for (int32_t y = 0; y < targetMipHeight; ++y)
		{
			for (int32_t x = 0; x < targetMipWidth; ++x)
			{
				int32_t fx = x << 1;
				int32_t fy = y << 1;

				uint16_t from[] =
				{
					m_depth[i - 1][fx + fy * fromMipWidth],
					m_depth[i - 1][fx + 1 + fy * fromMipWidth],
					m_depth[i - 1][fx + (fy + 1) * fromMipWidth],
					m_depth[i - 1][fx + 1 + (fy + 1) * fromMipWidth]
				};

				uint16_t d =
					min(
						min(from[0], from[1]),
						min(from[2], from[3])
					);

				m_depth[i][x + y * targetMipWidth] = d;
			}
		}
	}
}

void WorldCullingSwRaster::placeOccluder(const OccluderMesh* mesh, const Transform& transform)
{
	uint32_t vertexCount = mesh->getVertexCount();
	const float* vertices = mesh->getVertices();

	uint32_t indexCount = mesh->getIndexCount();
	const uint16_t* indices = mesh->getIndices();

	// Transform vertices into clip space.
	T_ASSERT (vertexCount < c_maxVertices);
	Matrix44 objectToClip = m_projection * m_view * transform.toMatrix44();
	for (uint32_t i = 0; i < vertexCount; ++i)
	{
		Vector4 Vo = Vector4::loadAligned(&vertices[i * 4]);
		m_clipVertices[i] = objectToClip * Vo;
	}

	// Clip, project and raster triangles.
	FragmentVisitor fragmentVisitor;
	fragmentVisitor.depth = m_depth[0].ptr();

	StaticVector< Triangle, 8 > triangles;
	for (uint32_t i = 0; i < indexCount; i += 3)
	{
		Triangle T =
		{
			m_clipVertices[indices[i + 0]],
			m_clipVertices[indices[i + 1]],
			m_clipVertices[indices[i + 2]]
		};

		triangles.clear();
		triangles.push_back(T);

		clipTriangles(triangles);

		for (uint32_t j = 0; j < triangles.size(); ++j)
		{
			const Triangle& Tc = triangles[j];

			const Vector4& Vc0 = Tc.v[0];
			const Vector4& Vc1 = Tc.v[1];
			const Vector4& Vc2 = Tc.v[2];

			Scalar iw0 = c_one / Vc0.w();
			Scalar iw1 = c_one / Vc1.w();
			Scalar iw2 = c_one / Vc2.w();

			Vector2 tv0(
				(Vc0.x() * iw0 * c_half + c_half) * c_width,
				(c_half - Vc0.y() * iw0 * c_half) * c_height
			);

			Vector2 tv1(
				(Vc1.x() * iw1 * c_half + c_half) * c_width,
				(c_half - Vc1.y() * iw1 * c_half) * c_height
			);

			Vector2 tv2(
				(Vc2.x() * iw2 * c_half + c_half) * c_width,
				(c_half - Vc2.y() * iw2 * c_half) * c_height
			);

			fragmentVisitor.iw0 = iw0;
			fragmentVisitor.iw1 = iw1;
			fragmentVisitor.iw2 = iw2;

			triangle(tv0, tv1, tv2, fragmentVisitor);
		}
	}
}

bool WorldCullingSwRaster::queryAabb(const Aabb3& aabb, const Transform& transform) const
{
	Vector4 extents[8];
	aabb.getExtents(extents);

	Vector4 mn(
		std::numeric_limits< float >::max(),
		std::numeric_limits< float >::max(),
		std::numeric_limits< float >::max(),
		std::numeric_limits< float >::max()
	);
	Vector4 mx(
		-std::numeric_limits< float >::max(),
		-std::numeric_limits< float >::max(),
		-std::numeric_limits< float >::max(),
		-std::numeric_limits< float >::max()
	);

	Matrix44 worldView = m_view * transform.toMatrix44();
	Matrix44 worldViewProj = m_projection * worldView;

	Scalar mniw(0.0f);

	for (int i = 0; i < sizeof_array(extents); ++i)
	{
		Vector4 p = worldViewProj * extents[i];
		if (p.w() <= 0.0f)
		{
			// Bounding box clipped to view plane; assume it's visible.
			return true;
		}

		Scalar iw = c_one / p.w();
		mniw = max(iw, mniw);

		// Homogeneous divide.
		p *= Scalar(iw);

		// Track screen space extents.
		mn = min(mn, p);
		mx = max(mx, p);
	}

	// Bias comparison in order to prevent false negative.
	mniw += Scalar(2.0f / 65535.0f);

	Vector4 ex = (mx - mn) * Vector4(float(c_width), float(c_height), 0.0f, 0.0f);
	int32_t w = int32_t(ex.x());
	int32_t h = int32_t(ex.y());
	int32_t mip = log2((min(w, h) + 1) / 2);

	if (mip > 0)
		--mip;

	w >>= 1;
	h >>= 1;

	int32_t mipCount = log2(max(c_width, c_height)) + 1;
	mip = min(mipCount - 1, mip);

	int32_t mipWidth = max(c_width >> mip, 1);
	int32_t mipHeight = max(c_height >> mip, 1);

	int32_t x0 = int32_t(( mn.x() * 0.5f + 0.5f) * mipWidth);
	int32_t y0 = int32_t((-mx.y() * 0.5f + 0.5f) * mipHeight);
	int32_t x1 = int32_t(( mx.x() * 0.5f + 0.5f) * mipWidth);
	int32_t y1 = int32_t((-mn.y() * 0.5f + 0.5f) * mipHeight);

	x0 = clamp(x0, 0, mipWidth - 1);
	y0 = clamp(y0, 0, mipWidth - 1);
	x1 = clamp(x1, 0, mipWidth - 1);
	y1 = clamp(y1, 0, mipWidth - 1);

	uint16_t df = uint16_t(mniw * 65535.0f);

	for (int32_t iy = y0; iy <= y1; ++iy)
	{
		for (int32_t ix = x0; ix <= x1; ++ix)
		{
			if (df >= m_depth[mip][ix + iy * mipWidth])
				return true;
		}
	}

	return false;
}

	}
}
