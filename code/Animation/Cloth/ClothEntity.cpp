/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Cloth/ClothEntity.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"

namespace traktor
{
	namespace animation
	{
		namespace
		{

struct ClothVertex
{
	float position[4];
	float normal[4];
	float texCoord[2];
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.animation.ClothEntity", ClothEntity, world::Entity)

ClothEntity::ClothEntity()
:	m_time(0.0f)
,	m_updateTime(0.0f)
,	m_scale(0.0f)
,	m_damping(1.0f)
,	m_solverIterations(0)
,	m_resolutionX(0)
,	m_resolutionY(0)
,	m_triangleCount(0)
,	m_updateRequired(true)
{
}

ClothEntity::~ClothEntity()
{
}

bool ClothEntity::create(
	render::IRenderSystem* renderSystem,
	const resource::Proxy< render::Shader >& shader,
	uint32_t resolutionX,
	uint32_t resolutionY,
	float scale,
	float damping,
	uint32_t solverIterations
)
{
	Vector4 positionBase(-scale / 2.0f, scale / 2.0f, 0.0f, 1.0f);
	Vector4 positionScale(scale / resolutionX, -scale / resolutionX, 0.0f, 0.0f);

	m_nodes.resize(resolutionX * resolutionY);
	for (uint32_t y = 0; y < resolutionY; ++y)
	{
		for (uint32_t x = 0; x < resolutionX; ++x)
		{
			m_nodes[x + y * resolutionX].position[0] =
			m_nodes[x + y * resolutionX].position[1] = Vector4(float(x), float(y), 0.0f, 0.0f) * positionScale + positionBase;
			m_nodes[x + y * resolutionX].texCoord = Vector2(float(x) / (resolutionX - 1), float(y) / (resolutionY - 1));
			m_nodes[x + y * resolutionX].invMass = Scalar(1.0f);
		}
	}

	uint32_t quadsX = resolutionX - 1;
	uint32_t quadsY = resolutionY - 1;
	Edge edge;

	for (uint32_t y = 0; y < resolutionY; ++y)
	{
		for (uint32_t x = 0; x < quadsX; ++x)
		{
			edge.index[0] = y * resolutionX + x;
			edge.index[1] = y * resolutionX + x + 1;
			m_edges.push_back(edge);
		}
	}

	for (uint32_t x = 0; x < resolutionX; ++x)
	{
		for (uint32_t y = 0; y < quadsY; ++y)
		{
			edge.index[0] = y * resolutionX + x;
			edge.index[1] = y * resolutionX + x + resolutionX;
			m_edges.push_back(edge);
		}
	}

	for (uint32_t y = 0; y < quadsY; ++y)
	{
		for (uint32_t x = 0; x < quadsX; ++x)
		{
			edge.index[0] = y * resolutionX + x;
			edge.index[1] = y * resolutionX + x + resolutionX + 1;
			m_edges.push_back(edge);

			edge.index[0] = y * resolutionX + x + 1;
			edge.index[1] = y * resolutionX + x + resolutionX;
			m_edges.push_back(edge);
		}
	}

	for (AlignedVector< Edge >::iterator i = m_edges.begin(); i != m_edges.end(); ++i)
		i->length = (m_nodes[i->index[0]].position[0] - m_nodes[i->index[1]].position[0]).xyz0().length();

	m_solverIterations = solverIterations;

	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat4, offsetof(ClothVertex, position)));
	vertexElements.push_back(render::VertexElement(render::DuNormal, render::DtFloat4, offsetof(ClothVertex, normal)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat2, offsetof(ClothVertex, texCoord)));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, /*2 * */resolutionX * resolutionY * sizeof(ClothVertex), true);
	if (!m_vertexBuffer)
		return false;

	m_resolutionX = resolutionX;
	m_resolutionY = resolutionY;
	m_triangleCount = quadsX * quadsY * 2/* * 2*/;

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, m_triangleCount * 3 * sizeof(uint16_t), false);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = static_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT (index);

	for (uint32_t y = 0; y < quadsY; ++y)
	{
		for (uint32_t x = 0; x < quadsX; ++x)
		{
			*index++ = x + y * resolutionX;
			*index++ = x + y * resolutionX + 1;
			*index++ = x + y * resolutionX + resolutionX;

			*index++ = x + y * resolutionX + 1;
			*index++ = x + y * resolutionX + 1 + resolutionX;
			*index++ = x + y * resolutionX + resolutionX;
		}
	}

	m_indexBuffer->unlock();

	m_scale = scale;
	m_damping = Scalar(1.0f - damping);
	m_shader = shader;
	return true;
}

void ClothEntity::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	if (!m_shader->hasTechnique(worldRenderPass.getTechnique()))
		return;

	if (m_updateRequired || !m_vertexBuffer->isContentValid())
	{
		ClothVertex* vertexFront = static_cast< ClothVertex* >(m_vertexBuffer->lock());
		T_ASSERT (vertexFront);

		for (uint32_t y = 0; y < m_resolutionY; ++y)
		{
			for (uint32_t x = 0; x < m_resolutionX; ++x)
			{
				uint32_t offset = x + y * m_resolutionX;
				const Node& node = m_nodes[offset];
				const Vector4& p = node.position[0];

				Vector4 nx = x < (m_resolutionX - 1) ? m_nodes[offset + 1].position[0] : p;
				Vector4 px = x > 0 ? m_nodes[offset - 1].position[0] : p;

				Vector4 ny = y < (m_resolutionY - 1) ? m_nodes[offset + m_resolutionX].position[0] : p;
				Vector4 py = y > 0 ? m_nodes[offset - m_resolutionX].position[0] : p;

				Vector4 nf = cross(ny - py, nx - px).normalized();

				p.storeUnaligned(vertexFront->position);
				nf.storeUnaligned(vertexFront->normal);
				vertexFront->texCoord[0] = node.texCoord.x;
				vertexFront->texCoord[1] = node.texCoord.y;
				vertexFront++;
			}
		}

		m_vertexBuffer->unlock();
		m_updateRequired = false;
	}

	worldRenderPass.setShaderTechnique(m_shader);
	worldRenderPass.setShaderCombination(
		m_shader,
		m_transform,
		m_aabb
	);

	render::RenderContext* renderContext = worldContext.getRenderContext();
	T_ASSERT (renderContext);

	render::IndexedRenderBlock* renderBlock = renderContext->alloc< render::IndexedRenderBlock >();

	renderBlock->distance = 0.0f;
	renderBlock->program = m_shader->getCurrentProgram();
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitive = render::PtTriangles;
	renderBlock->offset = 0;
	renderBlock->count = m_triangleCount;
	renderBlock->minIndex = 0;
	renderBlock->maxIndex = uint32_t(m_nodes.size() * 2 - 1);

	renderBlock->programParams->beginParameters(renderContext);
	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		m_transform,
		m_transform,
		m_aabb
	);
	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(
		m_shader->getCurrentPriority(),
		renderBlock
	);
}

void ClothEntity::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
}

void ClothEntity::setTransform(const Transform& transform)
{
	m_transform = transform;
}

bool ClothEntity::getTransform(Transform& outTransform) const
{
	outTransform = m_transform;
	return true;
}

Aabb3 ClothEntity::getBoundingBox() const
{
	return m_aabb;
}

void ClothEntity::update(const world::UpdateParams& update)
{
#if !TARGET_OS_IPHONE
	const float c_updateDeltaTime = 1.0f / 30.0f;
#else
	const float c_updateDeltaTime = 1.0f / 10.0f;
#endif
	const float c_timeScale = 2.0f;
	const Scalar c_stiffness(0.0025f);

	Transform transformInv = m_transform.inverse();
	Vector4 gravity = transformInv * Vector4(0.0f, -0.5f, 0.0f, 0.0f);
	Vector4 wind = transformInv * Vector4(1.0f, 0.0f, Scalar(traktor::cosf(m_time * 0.25f) * 0.5f + 0.5f), 0.0f).normalized();

	for (m_time += update.deltaTime * c_timeScale; m_updateTime < m_time; m_updateTime += c_updateDeltaTime)
	{
		m_aabb = Aabb3();
		for (AlignedVector< Node >::iterator i = m_nodes.begin(); i != m_nodes.end(); ++i)
		{
			if (i->invMass < Scalar(FUZZY_EPSILON))
				continue;

			Vector4 force = gravity + wind + Vector4(
				traktor::sinf(i->position[0].x() * 2.0f * PI + m_time * 0.2f) * 0.2f,
				traktor::cosf(i->position[0].y() * 2.0f * PI + m_time * 0.2f) * 0.2f,
				0.0f,
				0.0f
			);

			Vector4 current = i->position[0];
			i->position[0] += (current - i->position[1]) * m_damping + force * i->invMass * Scalar(c_updateDeltaTime * c_updateDeltaTime);
			i->position[1] = current;

			m_aabb.contain(i->position[0]);
		}

		for (uint32_t i = 0; i < m_solverIterations; ++i)
		{
			// Satisfy edge lengths.
			for (AlignedVector< Edge >::iterator j = m_edges.begin(); j != m_edges.end(); ++j)
			{
				Vector4 delta = m_nodes[j->index[1]].position[0] - m_nodes[j->index[0]].position[0];
				Scalar deltaLength = delta.length();
				if (deltaLength > FUZZY_EPSILON)
				{
					Scalar diff = (deltaLength - j->length) / deltaLength;
					m_nodes[j->index[0]].position[0] += delta * diff * m_nodes[j->index[0]].invMass * Scalar(0.5f);
					m_nodes[j->index[1]].position[0] -= delta * diff * m_nodes[j->index[1]].invMass * Scalar(0.5f);
				}
			}

			// Satisfy cloth stiffness.
			if (m_resolutionX >= 3 && m_resolutionY >= 3)
			{
				for (uint32_t y = 1; y < m_resolutionY - 1; ++y)
				{
					for (uint32_t x = 1; x < m_resolutionX - 1; ++x)
					{
						Vector4& center = m_nodes[x + y * m_resolutionX].position[0];

						// X axis
						{
							const Vector4& xn = m_nodes[x - 1 + y * m_resolutionX].position[0];
							const Vector4& xp = m_nodes[x + 1 + y * m_resolutionX].position[0];

							Vector4 v1 = (xp - center).normalized();
							Vector4 v2 = (xn - center).normalized();

							Scalar phi = dot3(v1, v2);
							if (phi > -1.0f + FUZZY_EPSILON)
							{
								Vector4 v = (v1 + v2).normalized();
								Vector4 xc = (xp + xn) / Scalar(2.0f);
								Scalar distance = Plane(v, xc).distance(center);
								center -= v * distance * c_stiffness;
							}
						}

						// Y axis
						{
							const Vector4& yn = m_nodes[x + (y - 1) * m_resolutionX].position[0];
							const Vector4& yp = m_nodes[x + (y + 1) * m_resolutionX].position[0];

							Vector4 v1 = (yp - center).normalized();
							Vector4 v2 = (yn - center).normalized();

							Scalar phi = dot3(v1, v2);
							if (phi > -1.0f + FUZZY_EPSILON)
							{
								Vector4 v = (v1 + v2).normalized();
								Vector4 yc = (yp + yn) / Scalar(2.0f);
								Scalar distance = Plane(v, yc).distance(center);
								center -= v * distance * c_stiffness;
							}
						}
					}
				}
			}
		}

		m_updateRequired = true;
	}
}

void ClothEntity::reset()
{
	Vector4 positionBase(-m_scale / 2.0f, m_scale / 2.0f, 0.0f, 1.0f);
	Vector4 positionScale(m_scale / m_resolutionX, -m_scale / m_resolutionY, 0.0f, 0.0f);

	for (uint32_t y = 0; y < m_resolutionY; ++y)
	{
		for (uint32_t x = 0; x < m_resolutionX; ++x)
		{
			m_nodes[x + y * m_resolutionX].position[0] =
			m_nodes[x + y * m_resolutionX].position[1] = Vector4(float(x), float(y), 0.0f, 0.0f) * positionScale + positionBase;
		}
	}
}

void ClothEntity::setNodeInvMass(uint32_t x, uint32_t y, float invMass)
{
	uint32_t index = x + y * m_resolutionX;
	if (index < m_resolutionX * m_resolutionY)
		m_nodes[index].invMass = Scalar(invMass);
}

	}
}
