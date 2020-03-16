#include <limits>
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Heightfield/Heightfield.h"
#include "Render/IndexBuffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexBuffer.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/Terrain.h"
#include "World/Entity/ComponentEntity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const render::Handle s_handleReflectionMap(L"ReflectionMap");
const render::Handle s_handleReflectionMapAvailable(L"ReflectionMapAvailable");
const render::Handle s_handleReflectionEnable(L"ReflectionEnable");
const render::Handle s_handleTerrainAvailable(L"TerrainAvailable");
const render::Handle s_handleHeightfield(L"Heightfield");
const render::Handle s_handleWorldOrigin(L"WorldOrigin");
const render::Handle s_handleWorldExtent(L"WorldExtent");
const render::Handle s_handleEye(L"Eye");
const render::Handle s_handleShallowTint(L"ShallowTint");
const render::Handle s_handleReflectionTint(L"ReflectionTint");
const render::Handle s_handleShadowTint(L"ShadowTint");
const render::Handle s_handleDeepColor(L"DeepColor");
const render::Handle s_handleOpacity(L"Opacity");
const render::Handle s_handleWavesA(L"WavesA");
const render::Handle s_handleWavesB(L"WavesB");

#pragma pack(1)
struct OceanVertex
{
	float pos[2];
	float edge;
};
#pragma pack()

const int32_t c_gridSize = 128;
const int32_t c_gridInfSize = 16;
const int32_t c_gridCells = (c_gridSize - 1) * (c_gridSize - 1);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanComponent", OceanComponent, world::IEntityComponent)

OceanComponent::OceanComponent()
:	m_owner(0)
,	m_opacity(0.0f)
,	m_maxAmplitude(0.0f)
,	m_allowSSReflections(true)
{
}

OceanComponent::~OceanComponent()
{
	destroy();
}

bool OceanComponent::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanComponentData& data)
{
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DuPosition, render::DtFloat2, offsetof(OceanVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DuCustom, render::DtFloat1, offsetof(OceanVertex, edge)));

	m_vertexBuffer = renderSystem->createVertexBuffer(vertexElements, c_gridSize * c_gridSize * sizeof(OceanVertex), false);
	if (!m_vertexBuffer)
		return false;

	OceanVertex* vertex = reinterpret_cast< OceanVertex* >(m_vertexBuffer->lock());
	T_ASSERT(vertex);

	for (int32_t iz = 0; iz < c_gridSize; ++iz)
	{
		float fz = float(iz) * 2.0f / c_gridSize - 1.0f;

		float ez0 = clamp(1.0f - float(iz) / c_gridInfSize, 0.0f, 1.0f);
		float ez1 = clamp(1.0f - float(c_gridSize - 1 - iz) / c_gridInfSize, 0.0f, 1.0f);

		for (int32_t ix = 0; ix < c_gridSize; ++ix)
		{
			float fx = float(ix) * 2.0f / c_gridSize - 1.0f;

			float ex0 = clamp(1.0f - float(ix) / c_gridInfSize, 0.0f, 1.0f);
			float ex1 = clamp(1.0f - float(c_gridSize - 1 - ix) / c_gridInfSize, 0.0f, 1.0f);
			float f = std::pow(max(max(ex0, ex1), max(ez0, ez1)), 8);

			vertex->pos[0] = fx;
			vertex->pos[1] = fz;
			vertex->edge = lerp(300.0f, 20000.0f, f);

			vertex++;
		}
	}

	m_vertexBuffer->unlock();

	m_indexBuffer = renderSystem->createIndexBuffer(render::ItUInt16, c_gridCells * 6 * sizeof(uint16_t), false);
	if (!m_indexBuffer)
		return false;

	uint16_t* index = reinterpret_cast< uint16_t* >(m_indexBuffer->lock());
	T_ASSERT(index);

	for (int32_t iz = 0; iz < c_gridSize - 1; ++iz)
	{
		int32_t base = iz * c_gridSize;
		for (int32_t ix = 0; ix < c_gridSize - 1; ++ix)
		{
			int32_t quad[] =
			{
				base + ix,
				base + ix + 1,
				base + ix + 1 + c_gridSize,
				base + ix + c_gridSize
			};

			*index++ = uint16_t(quad[0]);
			*index++ = uint16_t(quad[1]);
			*index++ = uint16_t(quad[2]);

			*index++ = uint16_t(quad[2]);
			*index++ = uint16_t(quad[3]);
			*index++ = uint16_t(quad[0]);
		}
	}

	m_indexBuffer->unlock();

	m_primitives.setIndexed(render::PtTriangles, 0, c_gridCells * 2, 0, c_gridSize * c_gridSize - 1);

	if (!resourceManager->bind(data.m_shader, m_shader))
		return false;

	if (data.m_reflectionMap)
	{
		if (!resourceManager->bind(data.m_reflectionMap, m_reflectionMap))
			return false;
	}

	if (data.m_terrain)
	{
		if (!resourceManager->bind(data.m_terrain, m_terrain))
			return false;
	}

	m_shallowTint = data.m_shallowTint;
	m_reflectionTint = data.m_reflectionTint;
	m_shadowTint = data.m_shadowTint;
	m_deepColor = data.m_deepColor;
	m_opacity = data.m_opacity;
	m_maxAmplitude = 0.0f;
	m_allowSSReflections = data.m_allowSSReflections;

	for (int32_t i = 0; i < sizeof_array(m_wavesA); ++i)
	{
		m_wavesA[i] = Vector4(data.m_waves[i].center.x, data.m_waves[i].center.y, data.m_waves[i].rate, 0.0f);
		m_wavesB[i] = Vector4(data.m_waves[i].amplitude, data.m_waves[i].frequency, data.m_waves[i].phase, data.m_waves[i].pinch);
		m_maxAmplitude += std::abs(data.m_waves[i].amplitude);
	}

	return true;
}

void OceanComponent::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
	m_reflectionMap.clear();
	m_shader.clear();
}

void OceanComponent::setOwner(world::ComponentEntity* owner)
{
	m_owner = owner;
}

void OceanComponent::setTransform(const Transform& transform)
{
}

Aabb3 OceanComponent::getBoundingBox() const
{
	return Aabb3();
}

void OceanComponent::update(const world::UpdateParams& update)
{
}

void OceanComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	bool reflectionEnable
)
{
	if (!m_owner)
		return;

	Transform transform = m_owner->getTransform();

	const Matrix44& view = worldRenderView.getView();
	Matrix44 viewInv = view.inverse();
	Vector4 eye = viewInv.translation().xyz1();
	if (eye.y() < transform.translation().y())
		return;

	auto perm = worldRenderPass.getPermutation(m_shader);

	m_shader->setCombination(s_handleReflectionMapAvailable, (bool)m_reflectionMap, perm);
	m_shader->setCombination(s_handleReflectionEnable, reflectionEnable && m_allowSSReflections, perm);
	m_shader->setCombination(s_handleTerrainAvailable, m_terrain, perm);

	auto sp = m_shader->getProgram(perm);
	if (!sp)
		return;

	render::SimpleRenderBlock* renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"Ocean");

	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->indexBuffer = m_indexBuffer;
	renderBlock->vertexBuffer = m_vertexBuffer;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setVectorParameter(s_handleEye, eye);
	renderBlock->programParams->setVectorParameter(s_handleShallowTint, m_shallowTint);
	renderBlock->programParams->setVectorParameter(s_handleReflectionTint, m_reflectionTint);
	renderBlock->programParams->setVectorParameter(s_handleShadowTint, m_shadowTint);
	renderBlock->programParams->setVectorParameter(s_handleDeepColor, m_deepColor);
	renderBlock->programParams->setFloatParameter(s_handleOpacity, m_opacity);
	renderBlock->programParams->setVectorArrayParameter(s_handleWavesA, m_wavesA, sizeof_array(m_wavesA));
	renderBlock->programParams->setVectorArrayParameter(s_handleWavesB, m_wavesB, sizeof_array(m_wavesB));

	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		transform,
		transform,
		Aabb3()
	);

	if (m_reflectionMap)
		renderBlock->programParams->setTextureParameter(s_handleReflectionMap, m_reflectionMap);

	if (m_terrain)
	{
		const Vector4& worldExtent = m_terrain->getHeightfield()->getWorldExtent();
		Vector4 worldOrigin = -worldExtent * Scalar(0.5f);

		renderBlock->programParams->setTextureParameter(s_handleHeightfield, m_terrain->getHeightMap());
		renderBlock->programParams->setVectorParameter(s_handleWorldOrigin, worldOrigin);
		renderBlock->programParams->setVectorParameter(s_handleWorldExtent, worldExtent);
	}

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

	}
}
