#include "Render/PrimitiveRenderer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/VertexElement.h"
#include "Render/VertexBuffer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Core/Math/Const.h"
#include "Core/Math/Plane.h"
#include "Core/Misc/Endian.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const Guid c_guidPrimitiveShader(L"{5B786C6B-8818-A24A-BD1C-EE113B79BCE2}");
const int c_bufferCount = 32000;

enum ShaderId
{
	SiWire,
	SiSolid,
	SiTexture,
	SiWireDepth,
	SiSolidDepth,
	SiTextureDepth
};

static render::handle_t s_handles[6];
static render::handle_t s_textureHandle;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.PrimitiveRenderer", PrimitiveRenderer, Object)

#pragma pack(1)
struct Vertex
{
	float pos[4];
	float texCoord[2];
	uint32_t rgb;
	
	T_FORCE_INLINE void set(const Vector4& pos_, const Color& rgb_)
	{
		pos_.store(pos);
		rgb = rgb_;

#if defined(T_BIG_ENDIAN)
		swap8in32(rgb);
#endif
	}

	T_FORCE_INLINE void set(const Vector4& pos_, const Vector2& texCoord_, const Color& rgb_)
	{
		pos_.store(pos);
		texCoord[0] = texCoord_.x;
		texCoord[1] = texCoord_.y;
		rgb = rgb_;

#if defined(T_BIG_ENDIAN)
		swap8in32(rgb);
#endif
	}
};
#pragma pack()

PrimitiveRenderer::PrimitiveRenderer()
:	m_currentBuffer(0)
,	m_vertexStart(0)
,	m_vertex(0)
,	m_viewNearZ(1.0f)
,	m_viewWidth(1000.0f)
,	m_viewHeight(1000.0f)
{
	s_handles[0] = render::getParameterHandle(L"Wire");
	s_handles[1] = render::getParameterHandle(L"Solid");
	s_handles[2] = render::getParameterHandle(L"Texture");
	s_handles[3] = render::getParameterHandle(L"WireDepth");
	s_handles[4] = render::getParameterHandle(L"SolidDepth");
	s_handles[5] = render::getParameterHandle(L"TextureDepth");
	s_textureHandle = render::getParameterHandle(L"Texture");
}

bool PrimitiveRenderer::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem
)
{
	resource::Proxy< Shader > shader(c_guidPrimitiveShader);
	return create(resourceManager, renderSystem, shader);
}

bool PrimitiveRenderer::create(
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	const resource::Proxy< Shader >& shader
)
{
	m_shader = shader;
	if (!resourceManager->bind(m_shader))
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
	m_depthEnable.push_back(true);

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

void PrimitiveRenderer::pushDepthEnable(bool depthEnable)
{
	m_depthEnable.push_back(depthEnable);
}

void PrimitiveRenderer::popDepthEnable()
{
	m_depthEnable.pop_back();
	T_ASSERT (!m_depthEnable.empty());
}

void PrimitiveRenderer::setClipDistance(float nearZ)
{
	m_viewNearZ = nearZ;
}

void PrimitiveRenderer::drawLine(
	const Vector4& start,
	const Vector4& end,
	const Color& color
)
{
	if (int(m_vertex - m_vertexStart + 2) >= c_bufferCount)
		return;

	Vector4 v1 = m_worldViewProj * Vector4(start.x(), start.y(), start.z(), 1.0f);
	Vector4 v2 = m_worldViewProj * Vector4(  end.x(),   end.y(),   end.z(), 1.0f);

	uint8_t shaderId = m_depthEnable.back() ? SiWireDepth : SiWire;
	if (m_batches.empty() || m_batches.back().shaderId != shaderId || m_batches.back().primitives.type != PtLines)
	{
		Batch batch;
		batch.shaderId = shaderId;
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
	const Color& color
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

	uint8_t shaderId = m_depthEnable.back() ? SiSolidDepth : SiSolid;
	if (m_batches.empty() || m_batches.back().shaderId != shaderId || m_batches.back().primitives.type != PtTriangles)
	{
		Batch batch;
		batch.shaderId = shaderId;
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
	const Color& color
)
{
	if (int(m_vertex - m_vertexStart + 3) >= c_bufferCount)
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

	dx *= Scalar(1.0f - sharpness);
	dy *= Scalar(1.0f - sharpness);

	Scalar csdx = dx * cs1.w();
	Scalar csdy = dy * cs1.w();

	uint8_t shaderId = m_depthEnable.back() ? SiSolidDepth : SiSolid;
	if (m_batches.empty() || m_batches.back().shaderId != shaderId || m_batches.back().primitives.type != PtTriangles)
	{
		Batch batch;
		batch.shaderId = shaderId;
		batch.primitives = Primitives(PtTriangles, int(m_vertex - m_vertexStart), 0);
		m_batches.push_back(batch);
	}

	m_vertex->set(Vector4(cs1.x() - csdx, cs1.y() - csdy, cs1.z(), cs1.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs1.x() + csdx, cs1.y() + csdy, cs1.z(), cs1.w()), color);
	m_vertex++;

	m_vertex->set(Vector4(cs2.x(), cs2.y(), cs2.z(), cs2.w()), color);
	m_vertex++;

	m_batches.back().primitives.count++;
}

void PrimitiveRenderer::drawWireAabb(
	const Vector4& center,
	const Vector4& extent,
	const Color& color
)
{
	drawWireAabb(
		Aabb(
			center - extent * Scalar(0.5f),
			center + extent * Scalar(0.5f)
		),
		color
	);
}

void PrimitiveRenderer::drawWireAabb(
	const Aabb& aabb,
	const Color& color
)
{
	if (aabb.empty())
		return;

	Vector4 extents[8];
	aabb.getExtents(extents);

	const int* edges = Aabb::getEdges();
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
	const Color& color
)
{
	drawLine(vert1, vert2, color);
	drawLine(vert2, vert3, color);
	drawLine(vert3, vert1, color);
}

void PrimitiveRenderer::drawWireTriangle(
	const Vector4& vert1,
	const Color& color1,
	const Vector4& vert2,
	const Color& color2,
	const Vector4& vert3,
	const Color& color3
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
	const Color& color
)
{
	drawLine(vert1, vert2, color);
	drawLine(vert2, vert3, color);
	drawLine(vert3, vert4, color);
	drawLine(vert4, vert1, color);
}

void PrimitiveRenderer::drawWireQuad(
	const Vector4& vert1,
	const Color& color1,
	const Vector4& vert2,
	const Color& color2,
	const Vector4& vert3,
	const Color& color3,
	const Vector4& vert4,
	const Color& color4
)
{
	drawLine(vert1, vert2, color1);
	drawLine(vert2, vert3, color2);
	drawLine(vert3, vert4, color3);
	drawLine(vert4, vert1, color4);
}

void PrimitiveRenderer::drawWireCylinder(
	const Matrix44& frame,
	float radius,
	float length,
	const Color& color
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

void PrimitiveRenderer::drawSolidAabb(
	const Vector4& center,
	const Vector4& extent,
	const Color& color
)
{
	drawSolidAabb(Aabb(center - extent * Scalar(0.5f), center + extent * Scalar(0.5f)), color);
}

void PrimitiveRenderer::drawSolidAabb(
	const Aabb& aabb,
	const Color& color
)
{
	if (aabb.empty())
		return;

	Vector4 eyeCenter = m_worldView.inverseOrtho().translation();

	Vector4 extents[8];
	aabb.getExtents(extents);

	const int* faces = Aabb::getFaces();
	const Vector4* normals = Aabb::getNormals();

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
			lerp(Color(0, 0, 0, 0), color, phi * 0.5f + 0.5f)
		);
	}
}

void PrimitiveRenderer::drawSolidTriangle(
	const Vector4& vert1,
	const Vector4& vert2,
	const Vector4& vert3,
	const Color& color
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
	const Color& color1,
	const Vector4& vert2,
	const Color& color2,
	const Vector4& vert3,
	const Color& color3
)
{
	if (int(m_vertex - m_vertexStart + 3) >= c_bufferCount)
		return;

	Vector4 v1 = m_worldViewProj * Vector4(vert1.x(), vert1.y(), vert1.z(), 1.0f);
	Vector4 v2 = m_worldViewProj * Vector4(vert2.x(), vert2.y(), vert2.z(), 1.0f);
	Vector4 v3 = m_worldViewProj * Vector4(vert3.x(), vert3.y(), vert3.z(), 1.0f);

	uint8_t shaderId = m_depthEnable.back() ? SiSolidDepth : SiSolid;
	if (m_batches.empty() || m_batches.back().shaderId != shaderId || m_batches.back().primitives.type != PtTriangles)
	{
		Batch batch;
		batch.shaderId = shaderId;
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
	const Color& color
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
	const Color& color1,
	const Vector4& vert2,
	const Color& color2,
	const Vector4& vert3,
	const Color& color3,
	const Vector4& vert4,
	const Color& color4
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
	const Color& color,
	ITexture* texture
)
{
	if (int(m_vertex - m_vertexStart + 3) >= c_bufferCount)
		return;

	Vector4 v1 = m_worldViewProj * Vector4(vert1.x(), vert1.y(), vert1.z(), 1.0f);
	Vector4 v2 = m_worldViewProj * Vector4(vert2.x(), vert2.y(), vert2.z(), 1.0f);
	Vector4 v3 = m_worldViewProj * Vector4(vert3.x(), vert3.y(), vert3.z(), 1.0f);

	uint8_t shaderId = m_depthEnable.back() ? SiTextureDepth : SiTexture;
	if (m_batches.empty() || m_batches.back().shaderId != shaderId || m_batches.back().primitives.type != PtTriangles)
	{
		Batch batch;
		batch.shaderId = shaderId;
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
	const Color& color,
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
	const Color& colorSolid,
	const Color& colorHint
)
{
	if (minAngle > maxAngle)
		std::swap(minAngle, maxAngle);

	if (maxAngle - minAngle < FUZZY_EPSILON)
		return;

	float minAngleClamp = ceil(minAngle / angleStep) * angleStep;
	float maxAngleClamp = floor(maxAngle / angleStep) * angleStep;

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
	const Color& colorSolid,
	const Color& colorHint
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

	m_vertexStart =
	m_vertex = static_cast< Vertex* >(m_vertexBuffers[m_currentBuffer]->lock());
	if (!m_vertex)
		return false;

	m_projection.push_back(Matrix44::identity());
	m_view.push_back(Matrix44::identity());
	m_world.push_back(Matrix44::identity());
	m_depthEnable.push_back(true);

	updateTransforms();

	m_viewWidth = float(renderView->getViewport().width);
	m_viewHeight = float(renderView->getViewport().height);

	return true;
}

void PrimitiveRenderer::end(IRenderView* renderView)
{
	T_ASSERT (m_vertex);

	m_vertexBuffers[m_currentBuffer]->unlock();
	if (m_shader.validate())
	{
		renderView->setVertexBuffer(m_vertexBuffers[m_currentBuffer]);

		for (AlignedVector< Batch >::iterator i = m_batches.begin(); i != m_batches.end(); ++i)
		{
			m_shader->setTechnique(s_handles[i->shaderId]);
			if (i->texture)
				m_shader->setSamplerTexture(s_textureHandle, i->texture);
			m_shader->draw(renderView, i->primitives);
		}
	}

	m_currentBuffer = (m_currentBuffer + 1) % sizeof_array(m_vertexBuffers);

	m_vertexStart =
	m_vertex = 0;
	m_batches.resize(0);

	m_projection.resize(0);
	m_view.resize(0);
	m_world.resize(0);
	m_depthEnable.resize(0);
}

void PrimitiveRenderer::updateTransforms()
{
	m_worldView = m_view.back() * m_world.back();
	m_worldViewProj = m_projection.back() * m_worldView;
}

	}
}
