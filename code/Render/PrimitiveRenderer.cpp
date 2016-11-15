#include "Core/Math/Const.h"
#include "Core/Math/Half.h"
#include "Core/Math/Winding3.h"
#include "Core/Misc/Endian.h"
#include "Core/Thread/Acquire.h"
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

static handle_t s_handleProjection;
static handle_t s_handleDepthTest;
static handle_t s_handleDepthWrite;
static handle_t s_handleTexture;
static handle_t s_handleDepth;

		}

#pragma pack(1)
struct Vertex
{
	float pos[4];
	half_t texCoord[2];
	uint32_t rgb;
	
	T_FORCE_INLINE void set(const Vector4& pos_, const Color4ub& rgb_)
	{
		pos_.storeUnaligned(pos);
		rgb = rgb_.getABGR();
	}

	T_FORCE_INLINE void set(const Vector4& pos_, const Vector2& texCoord_, const Color4ub& rgb_)
	{
		pos_.storeUnaligned(pos);
		texCoord[0] = floatToHalf(texCoord_.x);
		texCoord[1] = floatToHalf(texCoord_.y);
		rgb = rgb_.getABGR();
	}
};
#pragma pack()

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PrimitiveRenderer", PrimitiveRenderer, Object)

PrimitiveRenderer::PrimitiveRenderer()
:	m_currentFrame(0)
,	m_viewNearZ(1.0f)
,	m_vertexHead(0)
,	m_vertexTail(0)
{
	s_handleProjection = getParameterHandle(L"Projection");
	s_handleDepthTest = getParameterHandle(L"DepthTest");
	s_handleDepthWrite = getParameterHandle(L"DepthWrite");
	s_handleTexture = getParameterHandle(L"Texture");
	s_handleDepth = getParameterHandle(L"Depth");
}

bool PrimitiveRenderer::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t frameCount
)
{
	return create(resourceManager, renderSystem, c_idPrimitiveShader, frameCount);
}

bool PrimitiveRenderer::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	const resource::Id< Shader >& shader,
	uint32_t frameCount
)
{
	if (!resourceManager->bind(shader, m_shader))
		return 0;

	m_renderSystem = renderSystem;
	m_frames.resize(frameCount);
	m_view.push_back(Matrix44::identity());
	m_world.push_back(Matrix44::identity());
	m_depthState.push_back(DepthState(true, false, false));

	updateTransforms();
	return true;
}

void PrimitiveRenderer::destroy()
{
	for (RefArray< VertexBuffer >::iterator i = m_freeVertexBuffers.begin(); i != m_freeVertexBuffers.end(); ++i)
		(*i)->destroy();
	m_freeVertexBuffers.clear();

	for (AlignedVector< Frame >::iterator i = m_frames.begin(); i != m_frames.end(); ++i)
	{
		for (RefArray< VertexBuffer >::iterator j = i->vertexBuffers.begin(); j != i->vertexBuffers.end(); ++j)
			(*j)->destroy();
	}
	m_frames.clear();
}

bool PrimitiveRenderer::begin(uint32_t frame, const Matrix44& projection)
{
	m_currentFrame = &m_frames[frame];
	m_currentFrame->vertexBuffers.resize(0);
	m_currentFrame->projections.resize(0);
	m_currentFrame->projections.push_back(projection);
	m_currentFrame->batches.resize(0);

	m_view.push_back(Matrix44::identity());
	m_world.push_back(Matrix44::identity());
	m_depthState.push_back(DepthState(true, false, false));

	updateTransforms();
	return true;
}

void PrimitiveRenderer::end(uint32_t frame)
{
	T_ASSERT (m_currentFrame == &m_frames[frame]);

	if (m_vertexHead)
	{
		m_currentFrame->vertexBuffers.back()->unlock();
		m_vertexHead =
		m_vertexTail = 0;
	}

	m_currentFrame = 0;

	m_view.resize(0);
	m_world.resize(0);
	m_depthState.resize(0);
}

void PrimitiveRenderer::render(IRenderView* renderView, uint32_t frame)
{
	Frame& f = m_frames[frame];

	for (AlignedVector< Batch >::iterator i = f.batches.begin(); i != f.batches.end(); ++i)
	{
		m_shader->setCombination(s_handleDepthTest, i->depthState.depthTest);
		m_shader->setCombination(s_handleDepthWrite, i->depthState.depthWrite);
		m_shader->setCombination(s_handleDepth, i->depthState.depthOutput);
		m_shader->setCombination(s_handleTexture, i->texture != 0);

		m_shader->setMatrixParameter(s_handleProjection, f.projections[i->projection]);

		if (i->texture)
			m_shader->setTextureParameter(s_handleTexture, i->texture);

		m_shader->draw(
			renderView,
			i->vertexBuffer,
			0,
			i->primitives
		);
	}

	// Return frame's vertex buffers into free list.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		m_freeVertexBuffers.insert(m_freeVertexBuffers.end(), f.vertexBuffers.begin(), f.vertexBuffers.end());
	}
	
	// Reset frame.
	f.vertexBuffers.resize(0);
	f.projections.resize(0);
	f.batches.resize(0);
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

void PrimitiveRenderer::pushDepthState(bool depthTest, bool depthWrite, bool depthOutput)
{
	m_depthState.push_back(DepthState(depthTest, depthWrite, depthOutput));
}

void PrimitiveRenderer::popDepthState()
{
	m_depthState.pop_back();
	T_ASSERT (!m_depthState.empty());
}

void PrimitiveRenderer::setProjection(const Matrix44& projection)
{
	m_currentFrame->projections.push_back(projection);
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
	Vertex* v = allocBatch(PtLines, 1, 0);
	if (!v)
		return;

	v++->set(m_worldView * start.xyz1(), color);
	v++->set(m_worldView * end.xyz1(), color);
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

	Vector4 cs1 = m_currentFrame->projections.back() * vs1;
	Vector4 cs2 = m_currentFrame->projections.back() * vs2;

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

	dx = (dx * Scalar(width)) / (dln * Scalar(500.0f));
	dy = (dy * Scalar(width)) / (dln * Scalar(500.0f));

	Scalar dx1 = dx * cs1.w();
	Scalar dy1 = dy * cs1.w();

	Scalar dx2 = dx * cs2.w();
	Scalar dy2 = dy * cs2.w();

	Vertex* v = allocBatch(PtTriangles, 2, 0);
	if (!v)
		return;

	v++->set(Vector4(vs1.x() - dx1, vs1.y() - dy1, vs1.z(), vs1.w()), color);
	v++->set(Vector4(vs1.x() + dx1, vs1.y() + dy1, vs1.z(), vs1.w()), color);
	v++->set(Vector4(vs2.x() - dx2, vs2.y() - dy2, vs2.z(), vs2.w()), color);
	v++->set(Vector4(vs1.x() + dx1, vs1.y() + dy1, vs1.z(), vs1.w()), color);
	v++->set(Vector4(vs2.x() + dx2, vs2.y() + dy2, vs2.z(), vs2.w()), color);
	v++->set(Vector4(vs2.x() - dx2, vs2.y() - dy2, vs2.z(), vs2.w()), color);
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

	Scalar diffuse = dot3(-dn, Vector4(0.0f, 1.0f, 0.0f)) * Scalar(0.25f) + Scalar(0.75f);
	uint8_t mcap = uint8_t(diffuse * 255);

	for (int32_t i = 0; i < 16; ++i)
	{
		float a0 = (i / 16.0f) * TWO_PI;
		float a1 = a0 + (1.0f / 16.0f) * TWO_PI;

		float u0 = cosf(a0);
		float v0 = sinf(a0);
		float u1 = cosf(a1);
		float v1 = sinf(a1);

		Winding3 w(
			start + u * Scalar(u0 * radius) + v * Scalar(v0 * radius),
			start + u * Scalar(u1 * radius) + v * Scalar(v1 * radius),
			end
		);

		Plane plane;
		w.getPlane(plane);

		Scalar diffuse = dot3(plane.normal(), Vector4(0.0f, 1.0f, 0.0f)) * Scalar(0.25f) + Scalar(0.75f);
		uint8_t m = uint8_t(diffuse * 255);

		drawSolidTriangle(
			w[0],
			w[1],
			w[2],
			color * Color4ub(m, m, m, 255)
		);

		drawSolidTriangle(
			w[0],
			w[1],
			start,
			color * Color4ub(mcap, mcap, mcap, 255)
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
	Vector4 cv = m_worldView * center.xyz1();
	Vector4 cc = m_currentFrame->projections.back() * cv;

	Scalar dx = cc.w() * Scalar(size / 500.0f);
	Scalar dy = cc.w() * Scalar(size / 500.0f);

	Vertex* v = allocBatch(PtTriangles, 2, 0);
	if (!v)
		return;

	v++->set(cv + Vector4(-dx, -dy, 0.0f), color);
	v++->set(cv + Vector4( dx, -dy, 0.0f), color);
	v++->set(cv + Vector4(-dx,  dy, 0.0f), color);
	v++->set(cv + Vector4( dx, -dy, 0.0f), color);
	v++->set(cv + Vector4( dx,  dy, 0.0f), color);
	v++->set(cv + Vector4(-dx,  dy, 0.0f), color);
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
	Vector4 v1 = m_worldView * vert1.xyz1();
	Vector4 v2 = m_worldView * vert2.xyz1();
	Vector4 v3 = m_worldView * vert3.xyz1();

	Vertex* v = allocBatch(PtTriangles, 1, 0);
	if (!v)
		return;

	v++->set(v1, color1);
	v++->set(v2, color2);
	v++->set(v3, color3);
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
	Vector4 v1 = m_worldView * vert1.xyz1();
	Vector4 v2 = m_worldView * vert2.xyz1();
	Vector4 v3 = m_worldView * vert3.xyz1();

	Vertex* v = allocBatch(PtTriangles, 1, texture);
	if (!v)
		return;

	v++->set(v1, texCoord1, color);
	v++->set(v2, texCoord2, color);
	v++->set(v3, texCoord3, color);
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

void PrimitiveRenderer::updateTransforms()
{
	m_worldView = m_view.back() * m_world.back();
}

Vertex* PrimitiveRenderer::allocBatch(render::PrimitiveType primitiveType, uint32_t primitiveCount, render::ITexture* texture)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	const uint32_t c_primitiveMul[] = { 1, 0, 2, 0, 3 };

	uint32_t vertexCount = primitiveCount * c_primitiveMul[primitiveType];
	if (!vertexCount)
		return 0;

	// Check if enough room is available in current vertex buffer.
	if (m_vertexHead)
	{
		uint32_t vertexCountAvail = c_bufferCount - uint32_t(m_vertexTail - m_vertexHead);
		if (vertexCount > vertexCountAvail)
		{
			// Not enough room, finish of current vertex buffer.
			m_currentFrame->vertexBuffers.back()->unlock();
			m_vertexHead =
			m_vertexTail = 0;
		}
	}

	if (!m_vertexHead)
	{
		if (m_freeVertexBuffers.empty())
		{
			// No free buffers; need to allocate a new buffer.
			std::vector< VertexElement > vertexElements;
			vertexElements.push_back(VertexElement(DuPosition, DtFloat4, offsetof(Vertex, pos), 0));
			vertexElements.push_back(VertexElement(DuCustom, DtHalf2, offsetof(Vertex, texCoord), 0));
			vertexElements.push_back(VertexElement(DuColor, DtByte4N, offsetof(Vertex, rgb), 0));
			T_ASSERT (getVertexSize(vertexElements) == sizeof(Vertex));

			Ref< render::VertexBuffer > vertexBuffer = m_renderSystem->createVertexBuffer(vertexElements, c_bufferCount * sizeof(Vertex), true);
			if (!vertexBuffer)
				return 0;

			m_freeVertexBuffers.push_back(vertexBuffer);
		}
		T_ASSERT (!m_freeVertexBuffers.empty());

		// Pick buffer from free list.
		Ref< render::VertexBuffer > vertexBuffer = m_freeVertexBuffers.back();
		m_freeVertexBuffers.pop_back();

		// Lock new buffer.
		m_vertexHead =
		m_vertexTail = (Vertex*)vertexBuffer->lock();
		if (!m_vertexHead)
			return 0;

		// Place buffer last in frame's used list.
		m_currentFrame->vertexBuffers.push_back(vertexBuffer);
	}

	// Create new batch if necessary.
	AlignedVector< Batch >& batches = m_currentFrame->batches;
	uint32_t projection = m_currentFrame->projections.size() - 1;
	if (
		batches.empty() ||
		batches.back().projection != projection ||
		batches.back().depthState != m_depthState.back() ||
		batches.back().vertexBuffer != m_currentFrame->vertexBuffers.back() ||
		batches.back().texture != texture ||
		batches.back().primitives.type != primitiveType
	)
	{
		Batch batch;
		batch.projection = projection;
		batch.depthState = m_depthState.back();
		batch.vertexBuffer = m_currentFrame->vertexBuffers.back();
		batch.texture = texture;
		batch.primitives = Primitives(primitiveType, uint32_t(m_vertexTail - m_vertexHead), 0);
		batches.push_back(batch);
	}
	batches.back().primitives.count += primitiveCount;

	// Increment vertex pointer, return where caller must write vertices.
	Vertex* vertexPtr = m_vertexTail;
	m_vertexTail += vertexCount;
	return vertexPtr;
}

	}
}
