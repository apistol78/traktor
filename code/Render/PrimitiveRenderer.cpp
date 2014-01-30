#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/Endian.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/PrimitiveRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const resource::Id< Shader > c_idPrimitiveShader(Guid(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}"));
const int c_bufferCount = 16 * 1024;

enum ShaderId
{
	SiWire = 0,
	SiSolid = 1,
	SiTexture = 2
};

static render::handle_t s_handles[6];
static render::handle_t s_handleDepthTest;
static render::handle_t s_handleDepthWrite;
static render::handle_t s_handleTexture;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PrimitiveRenderer", PrimitiveRenderer, Object)

#pragma pack(1)
struct Vertex
{
	float pos[4];
	float texCoord[2];
	uint32_t rgb;
	
	T_FORCE_INLINE void set(const Vector4& pos_, const Color4ub& rgb_)
	{
		pos_.storeUnaligned(pos);
		rgb = rgb_.getABGR();
	}

	T_FORCE_INLINE void set(const Vector4& pos_, const Vector2& texCoord_, const Color4ub& rgb_)
	{
		pos_.storeUnaligned(pos);
		texCoord[0] = texCoord_.x;
		texCoord[1] = texCoord_.y;
		rgb = rgb_.getABGR();
	}
};
#pragma pack()

PrimitiveRenderer::PrimitiveRenderer()
:	m_renderView(0)
,	m_currentBuffer(0)
,	m_vertexStart(0)
,	m_vertex(0)
,	m_viewNearZ(1.0f)
,	m_viewWidth(1000.0f)
,	m_viewHeight(1000.0f)
{
	s_handles[0] = render::getParameterHandle(L"Wire");
	s_handles[1] = render::getParameterHandle(L"Solid");
	s_handles[2] = render::getParameterHandle(L"Texture");
	s_handleDepthTest = render::getParameterHandle(L"DepthTest");
	s_handleDepthWrite = render::getParameterHandle(L"DepthWrite");
	s_handleTexture = render::getParameterHandle(L"Texture");
}

bool PrimitiveRenderer::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem
)
{
	return create(resourceManager, renderSystem, c_idPrimitiveShader);
}

bool PrimitiveRenderer::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	const resource::Id< Shader >& shader
)
{
	if (!resourceManager->bind(shader, m_shader))
		return 0;

	std::vector< VertexElement > vertexElements;
	vertexElements.push_back(VertexElement(
		DuPosition,
		DtFloat4,
		offsetof(Vertex, pos),
		0
	));
	vertexElements.push_back(VertexElement(
		DuCustom,
		DtFloat2,
		offsetof(Vertex, texCoord),
		0
	));
	vertexElements.push_back(VertexElement(
		DuColor,
		DtByte4N,
		offsetof(Vertex, rgb),
		0
	));
	
	for (int i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		m_vertexBuffers[i] = renderSystem->createVertexBuffer(
			vertexElements,
			c_bufferCount * sizeof(Vertex),
			true
		);
		if (!m_vertexBuffers[i])
			return false;
	}

	m_vertexStart =
	m_vertex = 0;

	m_projection.push_back(Matrix44::identity());
	m_view.push_back(Matrix44::identity());
	m_world.push_back(Matrix44::identity());
	m_depthState.push_back(std::make_pair(true, false));

	updateTransforms();
	return true;
}

void PrimitiveRenderer::destroy()
{
	if (m_vertex)
	{
		m_vertexBuffers[m_currentBuffer]->unlock();
		m_vertexStart =
		m_vertex = 0;
	}

	for (int i = 0; i < sizeof_array(m_vertexBuffers); ++i)
	{
		if (m_vertexBuffers[i])
		{
			m_vertexBuffers[i]->destroy();
			m_vertexBuffers[i] = 0;
		}
	}
}

void PrimitiveRenderer::pushProjection(const Matrix44& projection)
{
	m_projection.push_back(projection);
	updateTransforms();
}

void PrimitiveRenderer::popProjection()
{
	m_projection.pop_back();
	T_ASSERT (!m_projection.empty());
	updateTransforms();
}

void PrimitiveRenderer::pushView(const Matrix44& view)
{
	m_view.push_back(view);
	updateTransforms();
}

void PrimitiveRenderer::popView()
{
	m_view.pop_back();
	T_ASSERT (!m_view.empty());
	updateTransforms();
}

void PrimitiveRenderer::pushWorld(const Matrix44& transform)
{
	m_world.push_back(transform);
	updateTransforms();
}

void PrimitiveRenderer::popWorld()
{
	m_world.pop_back();
	T_ASSERT (!m_world.empty());
	updateTransforms();
}

void PrimitiveRenderer::pushDepthState(bool depthTest, bool depthWrite)
{
	m_depthState.push_back(std::make_pair(depthTest, depthWrite));
}

void PrimitiveRenderer::popDepthState()
{
	m_depthState.pop_back();
	T_ASSERT (!m_depthState.empty());
}

void PrimitiveRenderer::setClipDistance(float nearZ)
{
	m_viewNearZ = nearZ;
}

void PrimitiveRenderer::drawLine(
	const Vector4& start,
	const Vector4& end,
	const Color4ub& color
)
{
	if (int(m_vertex - m_vertexStart + 2) >= c_bufferCount)
		flush();

	Vector4 v1 = m_worldViewProj * start.xyz1();
	Vector4 v2 = m_worldViewProj * end.xyz1();

	if (
		m_batches.empty() ||
		m_batches.back().shaderId != SiWire ||
		m_batches.back().depthState != m_depthState.back() ||
		m_batches.back().primitives.type != PtLines
	)
	{
		Batch batch;
		batch.shaderId = SiWire;
		batch.depthState = m_depthState.back();
		batch.primitives = Primitives(PtLines, int(m_vertex - m_vertexStart), 0);
		m_batches.push_back(batch);
	}

	m_vertex->set(v1, color);
	m_vertex++;

	m_vertex->set(v2, color);
	m_vertex++;

	m_batches.back().primitives.count++;
}

void PrimitiveRenderer::drawLine(
	const Vector4& start,
	const Vector4& end,
	float width,
	const Color4ub& color
)
{
	if (width < 2.0f - FUZZY_EPSILON)
	{
		drawLine(start, end, color);
		return;
	}

	if (int(m_vertex - m_vertexStart + 6) >= c_bufferCount)
		return;

	Vector4 vs1 = m_worldView * start.xyz1();
	Vector4 vs2 = m_worldView * end.xyz1();

	const Plane vp(0.0f, 0.0f, 1.0f, -m_viewNearZ);

	bool i1 = bool(vp.distance(vs1) >= 0.0f);
	bool i2 = bool(vp.distance(vs2) >= 0.0f);
	if (!i1 && !i2)
		return;

	if (i1 != i2)
	{
		Vector4 vsc;
		Scalar k;

		if (vp.segmentIntersection(vs1, vs2, k, &vsc))
		{
			if (!i1)
				vs1 = vsc;
			else
				vs2 = vsc;
		}
	}

	Vector4 cs1 = m_projection.back() * vs1;
	Vector4 cs2 = m_projection.back() * vs2;

	Scalar cw1 = cs1.w(), cw2 = cs2.w();
	if (cw1 <= Scalar(FUZZY_EPSILON) || cw2 <= Scalar(FUZZY_EPSILON))
		return;

	Scalar sx1 = cs1.x() / cw1;
	Scalar sy1 = cs1.y() / cw1;

	Scalar sx2 = cs2.x() / cw2;
	Scalar sy2 = cs2.y() / cw2;

	Scalar dy =   sx2 - sx1;
	Scalar dx = -(sy2 - sy1);

	Scalar dln = Scalar(sqrtf(dx * dx + dy * dy));
	if (dln <= FUZZY_EPSILON)
		return;

	dx = (dx * Scalar(width)) / (dln * Scalar(m_viewWidth));
	dy = (dy * Scalar(width)) / (dln * Scalar(m_viewHeight));

	Scalar dx1 = dx * cs1.w();
	Scalar dy1 = dy * cs1.w();

	Scalar dx2 = dx * cs2.w();
	Scalar dy2 = dy * cs2.w();

	if (
		m_batches.empty() ||
		m_batches.back().shaderId != SiSolid ||
		m_batches.back().depthState != m_depthState.back() ||
		m_batches.back().primitives.type != PtTriangles
	)
	{
		Batch batch;
		batch.shaderId = SiSolid;
		batch.depthState = m_depthState.back();
		batch.primitives = Primitives(PtTriangles, int(m_vertex - m_vertexStart), 0);
		m_batches.push_back(batch);
	}

	m_vertex->set(Vector4(cs1.x() - dx1, cs1.y() - dy1, cs1.z(), cs1.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs1.x() + dx1, cs1.y() + dy1, cs1.z(), cs1.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs2.x() - dx2, cs2.y() - dy2, cs2.z(), cs2.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs1.x() + dx1, cs1.y() + dy1, cs1.z(), cs1.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs2.x() + dx2, cs2.y() + dy2, cs2.z(), cs2.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs2.x() - dx2, cs2.y() - dy2, cs2.z(), cs2.w()), color);
	m_vertex++;

	m_batches.back().primitives.count += 2;
}

void PrimitiveRenderer::drawArrowHead(
	const Vector4& start,
	const Vector4& end,
	float sharpness,
	const Color4ub& color
)
{
	Vector4 d = end - start;
	Vector4 dn = d.normalized();

	const Vector4 c_axis[] =
	{
		Vector4(0.0f, 1.0f, 0.0f),
		Vector4(1.0f, 0.0f, 0.0f),
		Vector4(0.0f, 1.0f, 0.0f)
	};

	Vector4 u = cross(dn, c_axis[majorAxis3(dn)]).normalized();
	Vector4 v = cross(u, dn).normalized();

	float radius = d.length() * (1.0f - sharpness);

	for (int32_t i = 0; i < 8; ++i)
	{
		float a0 = (i / 8.0f) * TWO_PI;
		float a1 = a0 + (1.0f / 8.0f) * TWO_PI;

		float u0 = cosf(a0);
		float v0 = sinf(a0);
		float u1 = cosf(a1);
		float v1 = sinf(a1);

		drawSolidTriangle(
			start + u * Scalar(u0 * radius) + v * Scalar(v0 * radius),
			start + u * Scalar(u1 * radius) + v * Scalar(v1 * radius),
			end,
			color
		);
	}
}

void PrimitiveRenderer::drawWireAabb(
	const Vector4& center,
	const Vector4& extent,
	const Color4ub& color
)
{
	drawWireAabb(
		Aabb3(
			center - extent * Scalar(0.5f),
			center + extent * Scalar(0.5f)
		),
		color
	);
}

void PrimitiveRenderer::drawWireAabb(
	const Aabb3& aabb,
	const Color4ub& color
)
{
	if (aabb.empty())
		return;

	Vector4 extents[8];
	aabb.getExtents(extents);

	const int* edges = Aabb3::getEdges();
	for (int i = 0; i < 12; ++i)
	{
		drawLine(
			extents[edges[i * 2 + 0]],
			extents[edges[i * 2 + 1]],
			color
		);
	}
}

void PrimitiveRenderer::drawWireTriangle(
	const Vector4& vert1,
	const Vector4& vert2,
	const Vector4& vert3,
	const Color4ub& color
)
{
	drawLine(vert1, vert2, color);
	drawLine(vert2, vert3, color);
	drawLine(vert3, vert1, color);
}

void PrimitiveRenderer::drawWireTriangle(
	const Vector4& vert1,
	const Color4ub& color1,
	const Vector4& vert2,
	const Color4ub& color2,
	const Vector4& vert3,
	const Color4ub& color3
)
{
	drawLine(vert1, vert2, color1);
	drawLine(vert2, vert3, color2);
	drawLine(vert3, vert1, color3);
}

void PrimitiveRenderer::drawWireQuad(
	const Vector4& vert1,
	const Vector4& vert2,
	const Vector4& vert3,
	const Vector4& vert4,
	const Color4ub& color
)
{
	drawLine(vert1, vert2, color);
	drawLine(vert2, vert3, color);
	drawLine(vert3, vert4, color);
	drawLine(vert4, vert1, color);
}

void PrimitiveRenderer::drawWireQuad(
	const Vector4& vert1,
	const Color4ub& color1,
	const Vector4& vert2,
	const Color4ub& color2,
	const Vector4& vert3,
	const Color4ub& color3,
	const Vector4& vert4,
	const Color4ub& color4
)
{
	drawLine(vert1, vert2, color1);
	drawLine(vert2, vert3, color2);
	drawLine(vert3, vert4, color3);
	drawLine(vert4, vert1, color4);
}

void PrimitiveRenderer::drawWireCircle(
	const Vector4& center,
	const Vector4& normal,
	float radius,
	float width,
	const Color4ub& color
)
{
	const Vector4 c_axis[] =
	{
		Vector4(0.0f, 1.0f, 0.0f),
		Vector4(1.0f, 0.0f, 0.0f),
		Vector4(0.0f, 1.0f, 0.0f)
	};

	Vector4 u = cross(normal, c_axis[majorAxis3(normal)]).normalized();
	Vector4 v = cross(u, normal).normalized();

	for (int32_t i = 0; i < 16; ++i)
	{
		float a0 = (i / 16.0f) * TWO_PI;
		float a1 = a0 + (1.0f / 16.0f) * TWO_PI;

		float u0 = cosf(a0);
		float v0 = sinf(a0);
		float u1 = cosf(a1);
		float v1 = sinf(a1);

		drawLine(
			center + u * Scalar(u0 * radius) + v * Scalar(v0 * radius),
			center + u * Scalar(u1 * radius) + v * Scalar(v1 * radius),
			width,
			color
		);
	}
}

void PrimitiveRenderer::drawWireSphere(
	const Matrix44& frame,
	float radius,
	const Color4ub& color
)
{
	Vector4 centerV = m_worldView * frame.translation().xyz1();

	int32_t nlat = int32_t(4.0f * 15.0f / centerV.z() + 1);
	if (nlat > 16)
		nlat = 16;

	for (int lat = 0; lat < nlat; ++lat)
	{
		float r = radius * sinf(PI * float(lat + 1) / (nlat + 1));
		float y = radius * cosf(PI * float(lat + 1) / (nlat + 1));

		float x1 = r, z1 = 0.0f;
		for (int lng = 1; lng <= 32; ++lng)
		{
			float x2 = r * cosf(TWO_PI * (lng / 32.0f));
			float z2 = r * sinf(TWO_PI * (lng / 32.0f));

			drawLine(
				frame * Vector4(x1, y, z1, 1.0f),
				frame * Vector4(x2, y, z2, 1.0f),
				color
			);

			drawLine(
				frame * Vector4(x1, z1, y, 1.0f),
				frame * Vector4(x2, z2, y, 1.0f),
				color
			);

			drawLine(
				frame * Vector4(y, x1, z1, 1.0f),
				frame * Vector4(y, x2, z2, 1.0f),
				color
			);

			x1 = x2;
			z1 = z2;
		}
	}
}

void PrimitiveRenderer::drawWireCylinder(
	const Matrix44& frame,
	float radius,
	float length,
	const Color4ub& color
)
{
	const Vector4 axisU = frame.axisX();
	const Vector4 axisV = frame.axisY();

	for (int i = 0; i < 16; ++i)
	{
		float a1 = 2.0f * PI * float(i) / 16.0f;
		float a2 = 2.0f * PI * float(i + 1.0f) / 16.0f;

		Vector4 v1 = axisU * Scalar(cosf(a1)) + axisV * Scalar(sinf(a1));
		Vector4 v2 = axisU * Scalar(cosf(a2)) + axisV * Scalar(sinf(a2));

		Vector4 c1 = frame.translation() + frame.axisZ() * Scalar(length / 2.0f);
		Vector4 c2 = frame.translation() - frame.axisZ() * Scalar(length / 2.0f);

		Vector4 p1_1 = c1 + v1 * Scalar(radius);
		Vector4 p2_1 = c1 + v2 * Scalar(radius);
		Vector4 p1_2 = c2 + v1 * Scalar(radius);
		Vector4 p2_2 = c2 + v2 * Scalar(radius);

		drawLine(p1_1, p2_1, 1.0f, color);
		drawLine(p1_2, p2_2, 1.0f, color);
		drawLine(p1_1, p1_2, 1.0f, color);
	}
}

void PrimitiveRenderer::drawWireFrame(
	const Matrix44& frame,
	float length
)
{
	pushWorld(getWorld() * frame);

	drawLine(Vector4::origo(), Vector4(length, 0.0f, 0.0f, 1.0f), 1.0f, Color4ub(255, 0, 0, 255));
	drawLine(Vector4::origo(), Vector4(0.0f, length, 0.0f, 1.0f), 1.0f, Color4ub(0, 255, 0, 255));
	drawLine(Vector4::origo(), Vector4(0.0f, 0.0f, length, 1.0f), 1.0f, Color4ub(0, 0, 255, 255));

	drawArrowHead(Vector4(length, 0.0f, 0.0f, 1.0f), Vector4(length * 1.2f, 0.0f, 0.0f, 1.0f), 0.8f, Color4ub(255, 0, 0, 255));
	drawArrowHead(Vector4(0.0f, length, 0.0f, 1.0f), Vector4(0.0f, length * 1.2f, 0.0f, 1.0f), 0.8f, Color4ub(0, 255, 0, 255));
	drawArrowHead(Vector4(0.0f, 0.0f, length, 1.0f), Vector4(0.0f, 0.0f, length * 1.2f, 1.0f), 0.8f, Color4ub(0, 0, 255, 255));

	popWorld();
}

void PrimitiveRenderer::drawSolidPoint(
	const Vector4& center,
	float size,
	const Color4ub& color
)
{
	if (int(m_vertex - m_vertexStart + 6) >= c_bufferCount)
		flush();

	Vector4 cv = m_worldView * center.xyz1();
	Vector4 cc = m_projection.back() * cv;

	Scalar dx = cc.w() * Scalar(size / m_viewWidth);
	Scalar dy = cc.w() * Scalar(size / m_viewHeight);

	if (
		m_batches.empty() ||
		m_batches.back().shaderId != SiSolid ||
		m_batches.back().depthState != m_depthState.back() ||
		m_batches.back().primitives.type != PtTriangles
	)
	{
		Batch batch;
		batch.shaderId = SiSolid;
		batch.depthState = m_depthState.back();
		batch.primitives = Primitives(PtTriangles, int(m_vertex - m_vertexStart), 0);
		m_batches.push_back(batch);
	}

	m_vertex->set(cc + Vector4(-dx, -dy, 0.0f), color);
	m_vertex++;

	m_vertex->set(cc + Vector4( dx, -dy, 0.0f), color);
	m_vertex++;

	m_vertex->set(cc + Vector4(-dx,  dy, 0.0f), color);
	m_vertex++;

	m_vertex->set(cc + Vector4( dx, -dy, 0.0f), color);
	m_vertex++;

	m_vertex->set(cc + Vector4( dx,  dy, 0.0f), color);
	m_vertex++;

	m_vertex->set(cc + Vector4(-dx,  dy, 0.0f), color);
	m_vertex++;

	m_batches.back().primitives.count += 2;
}

void PrimitiveRenderer::drawSolidAabb(
	const Vector4& center,
	const Vector4& extent,
	const Color4ub& color
)
{
	drawSolidAabb(Aabb3(center - extent * Scalar(0.5f), center + extent * Scalar(0.5f)), color);
}

void PrimitiveRenderer::drawSolidAabb(
	const Aabb3& aabb,
	const Color4ub& color
)
{
	if (aabb.empty())
		return;

	Vector4 eyeCenter = m_worldView.inverse().translation();

	Vector4 extents[8];
	aabb.getExtents(extents);

	const int* faces = Aabb3::getFaces();
	const Vector4* normals = Aabb3::getNormals();

	for (int i = 0; i < 6; ++i)
	{
		Vector4 faceCenter = (
			extents[faces[i * 4 + 0]] +
			extents[faces[i * 4 + 1]] +
			extents[faces[i * 4 + 2]] +
			extents[faces[i * 4 + 3]]
		) / Scalar(4.0f);

		Scalar phi = dot3((eyeCenter - faceCenter).normalized(), normals[i]);
		if (phi < 0.0f)
			continue;

		drawSolidQuad(
			extents[faces[i * 4 + 0]],
			extents[faces[i * 4 + 1]],
			extents[faces[i * 4 + 2]],
			extents[faces[i * 4 + 3]],
			lerp(Color4ub(0, 0, 0, 0), color, phi * 0.5f + 0.5f)
		);
	}
}

void PrimitiveRenderer::drawSolidTriangle(
	const Vector4& vert1,
	const Vector4& vert2,
	const Vector4& vert3,
	const Color4ub& color
)
{
	drawSolidTriangle(
		vert1, color,
		vert2, color,
		vert3, color
	);
}

void PrimitiveRenderer::drawSolidTriangle(
	const Vector4& vert1,
	const Color4ub& color1,
	const Vector4& vert2,
	const Color4ub& color2,
	const Vector4& vert3,
	const Color4ub& color3
)
{
	if (int(m_vertex - m_vertexStart + 3) >= c_bufferCount)
		flush();

	Vector4 v1 = m_worldViewProj * Vector4(vert1.x(), vert1.y(), vert1.z(), 1.0f);
	Vector4 v2 = m_worldViewProj * Vector4(vert2.x(), vert2.y(), vert2.z(), 1.0f);
	Vector4 v3 = m_worldViewProj * Vector4(vert3.x(), vert3.y(), vert3.z(), 1.0f);

	if (
		m_batches.empty() ||
		m_batches.back().shaderId != SiSolid ||
		m_batches.back().primitives.type != PtTriangles
	)
	{
		Batch batch;
		batch.shaderId = SiSolid;
		batch.depthState = m_depthState.back();
		batch.primitives = Primitives(PtTriangles, int(m_vertex - m_vertexStart), 0);
		m_batches.push_back(batch);
	}

	m_vertex->set(v1, color1);
	m_vertex++;

	m_vertex->set(v2, color2);
	m_vertex++;

	m_vertex->set(v3, color3);
	m_vertex++;

	m_batches.back().primitives.count++;
}

void PrimitiveRenderer::drawSolidQuad(
	const Vector4& vert1,
	const Vector4& vert2,
	const Vector4& vert3,
	const Vector4& vert4,
	const Color4ub& color
)
{
	drawSolidQuad(
		vert1, color,
		vert2, color,
		vert3, color,
		vert4, color
	);
}

void PrimitiveRenderer::drawSolidQuad(
	const Vector4& vert1,
	const Color4ub& color1,
	const Vector4& vert2,
	const Color4ub& color2,
	const Vector4& vert3,
	const Color4ub& color3,
	const Vector4& vert4,
	const Color4ub& color4
)
{
	drawSolidTriangle(vert1, color1, vert2, color2, vert3, color3);
	drawSolidTriangle(vert1, color1, vert3, color3, vert4, color4);
}

void PrimitiveRenderer::drawTextureTriangle(
	const Vector4& vert1,
	const Vector2& texCoord1,
	const Vector4& vert2,
	const Vector2& texCoord2,
	const Vector4& vert3,
	const Vector2& texCoord3,
	const Color4ub& color,
	ITexture* texture
)
{
	if (int(m_vertex - m_vertexStart + 3) >= c_bufferCount)
		flush();

	Vector4 v1 = m_worldViewProj * Vector4(vert1.x(), vert1.y(), vert1.z(), 1.0f);
	Vector4 v2 = m_worldViewProj * Vector4(vert2.x(), vert2.y(), vert2.z(), 1.0f);
	Vector4 v3 = m_worldViewProj * Vector4(vert3.x(), vert3.y(), vert3.z(), 1.0f);

	if (
		m_batches.empty() ||
		m_batches.back().shaderId != SiTexture ||
		m_batches.back().depthState != m_depthState.back() ||
		m_batches.back().texture != texture ||
		m_batches.back().primitives.type != PtTriangles
	)
	{
		Batch batch;
		batch.shaderId = SiTexture;
		batch.depthState = m_depthState.back();
		batch.texture = texture;
		batch.primitives = Primitives(PtTriangles, int(m_vertex - m_vertexStart), 0);
		m_batches.push_back(batch);
	}

	m_vertex->set(v1, texCoord1, color);
	m_vertex++;

	m_vertex->set(v2, texCoord2, color);
	m_vertex++;

	m_vertex->set(v3, texCoord3, color);
	m_vertex++;

	m_batches.back().primitives.count++;
}

void PrimitiveRenderer::drawTextureQuad(
	const Vector4& vert1,
	const Vector2& texCoord1,
	const Vector4& vert2,
	const Vector2& texCoord2,
	const Vector4& vert3,
	const Vector2& texCoord3,
	const Vector4& vert4,
	const Vector2& texCoord4,
	const Color4ub& color,
	ITexture* texture
)
{
	drawTextureTriangle(vert1, texCoord1, vert2, texCoord2, vert3, texCoord3, color, texture);
	drawTextureTriangle(vert1, texCoord1, vert3, texCoord3, vert4, texCoord4, color, texture);
}

void PrimitiveRenderer::drawProtractor(
	const Vector4& position,
	const Vector4& base,
	const Vector4& zero,
	float minAngle,
	float maxAngle,
	float angleStep,
	float radius,
	const Color4ub& colorSolid,
	const Color4ub& colorHint
)
{
	if (minAngle > maxAngle)
		std::swap(minAngle, maxAngle);

	if (maxAngle - minAngle < FUZZY_EPSILON)
		return;

	float minAngleClamp = std::ceil(minAngle / angleStep) * angleStep;
	float maxAngleClamp = std::floor(maxAngle / angleStep) * angleStep;

	float angle1 = minAngle;
	float angle2 = minAngleClamp;

	Vector4 vxb[] =
	{
		position,
		position + (base * Scalar(cosf(angle1)) + zero * Scalar(sinf(angle1))) * Scalar(radius),
		position + (base * Scalar(cosf(angle2)) + zero * Scalar(sinf(angle2))) * Scalar(radius)
	};

	drawSolidTriangle(
		vxb[0],
		vxb[1],
		vxb[2],
		colorSolid
	);

	drawLine(
		vxb[0],
		vxb[1],
		colorHint
	);

	for (float i = minAngleClamp; i <= maxAngleClamp - angleStep; i += angleStep)
	{
		angle1 = i;
		angle2 = i + angleStep;

		Vector4 vx[] =
		{
			position,
			position + (base * Scalar(cosf(angle1)) + zero * Scalar(sinf(angle1))) * Scalar(radius),
			position + (base * Scalar(cosf(angle2)) + zero * Scalar(sinf(angle2))) * Scalar(radius)
		};

		drawSolidTriangle(
			vx[0],
			vx[1],
			vx[2],
			colorSolid
		);

		drawLine(
			vx[0],
			vx[1],
			colorHint
		);
	}

	angle1 = maxAngleClamp;
	angle2 = maxAngle;

	Vector4 vxe[] =
	{
		position,
		position + (base * Scalar(cosf(angle1)) + zero * Scalar(sinf(angle1))) * Scalar(radius),
		position + (base * Scalar(cosf(angle2)) + zero * Scalar(sinf(angle2))) * Scalar(radius)
	};

	drawSolidTriangle(
		vxe[0],
		vxe[1],
		vxe[2],
		colorSolid
	);

	drawLine(
		vxe[0],
		vxe[1],
		colorHint
	);

	drawLine(
		vxe[0],
		vxe[2],
		colorHint
	);
}

void PrimitiveRenderer::drawCone(
	const Matrix44& frame,
	float angleX,
	float angleY,
	float length,
	const Color4ub& colorSolid,
	const Color4ub& colorHint
)
{
	Vector4 c1 = frame.axisZ() * Scalar(cosf(angleX / 2.0f));
	Vector4 c2 = frame.axisZ() * Scalar(cosf(angleY / 2.0f));

	Vector4 d1 = frame.axisX() * Scalar(sinf(angleX / 2.0f));
	Vector4 d2 = frame.axisY() * Scalar(sinf(angleY / 2.0f));

	Vector4 vx[32];
	for (int i = 0; i < 32; ++i)
	{
		float a = (i / 32.0f) * PI * 2.0f;
		float b1 = (cosf(a * 2.0f) + 1.0f) / 2.0f;
		float b2 = cosf(a);
		float b3 = sinf(a);

		Vector4 c = c1 * Scalar(b1) + c2 * Scalar(1.0f - b1);
		Vector4 d = d1 * Scalar(b2) + d2 * Scalar(b3);
		Vector4 e = c + d;

		vx[i] = frame.translation() + e * Scalar(length);
	}

	for (int i = 0; i < 32; ++i)
	{
		drawSolidTriangle(
			frame.translation(),
			vx[i],
			vx[(i + 1) & 31],
			colorSolid
		);
	}

	for (int i = 0; i < 32; ++i)
	{
		drawLine(
			frame.translation(),
			vx[i],
			colorHint
		);
	}
}

bool PrimitiveRenderer::begin(IRenderView* renderView)
{
	T_ASSERT (!m_vertex);

	m_renderView = renderView;
	T_ASSERT (m_renderView);

	m_vertexStart =
	m_vertex = static_cast< Vertex* >(m_vertexBuffers[m_currentBuffer]->lock());
	if (!m_vertex)
		return false;

	m_projection.push_back(Matrix44::identity());
	m_view.push_back(Matrix44::identity());
	m_world.push_back(Matrix44::identity());
	m_depthState.push_back(std::make_pair(true, false));

	updateTransforms();

	m_viewWidth = float(m_renderView->getViewport().width);
	m_viewHeight = float(m_renderView->getViewport().height);

	return true;
}

void PrimitiveRenderer::end()
{
	T_ASSERT (m_vertex);

	m_vertexBuffers[m_currentBuffer]->unlock();

	for (AlignedVector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
	{
		m_shader->setTechnique(s_handles[i->shaderId]);
		m_shader->setCombination(s_handleDepthTest, i->depthState.first);
		m_shader->setCombination(s_handleDepthWrite, i->depthState.second);
		if (i->texture)
			m_shader->setTextureParameter(s_handleTexture, i->texture);
		m_shader->draw(
			m_renderView,
			m_vertexBuffers[m_currentBuffer],
			0,
			i->primitives
		);
	}

	m_currentBuffer = (m_currentBuffer + 1) % sizeof_array(m_vertexBuffers);

	m_vertexStart =
	m_vertex = 0;
	m_batches.resize(0);

	m_projection.resize(0);
	m_view.resize(0);
	m_world.resize(0);
	m_depthState.resize(0);
}

void PrimitiveRenderer::flush()
{
	T_ASSERT (m_vertex);

	m_vertexBuffers[m_currentBuffer]->unlock();

	for (AlignedVector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
	{
		m_shader->setTechnique(s_handles[i->shaderId]);
		if (i->texture)
			m_shader->setTextureParameter(s_handleTexture, i->texture);
		m_shader->draw(
			m_renderView,
			m_vertexBuffers[m_currentBuffer],
			0,
			i->primitives
		);
	}

	m_currentBuffer = (m_currentBuffer + 1) % sizeof_array(m_vertexBuffers);
	m_batches.resize(0);

	m_vertexStart =
	m_vertex = static_cast< Vertex* >(m_vertexBuffers[m_currentBuffer]->lock());
}

void PrimitiveRenderer::updateTransforms()
{
	m_worldView = m_view.back() * m_world.back();
	m_worldViewProj = m_projection.back() * m_worldView;
}

	}
}
