#include <limits>
#include "Core/Math/Float.h"
#include "Core/Misc/SafeDestroy.h"
#include "Heightfield/Heightfield.h"
#include "Render/Buffer.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ISimpleTexture.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/VertexElement.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/OceanComponent.h"
#include "Terrain/OceanComponentData.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const render::Handle s_handleTerrain_Heightfield(L"Terrain_Heightfield");
const render::Handle s_handleTerrain_WorldOrigin(L"Terrain_WorldOrigin");
const render::Handle s_handleTerrain_WorldExtent(L"Terrain_WorldExtent");
const render::Handle s_handleOcean_HaveTerrain(L"Ocean_HaveTerrain");
const render::Handle s_handleOcean_Eye(L"Ocean_Eye");
const render::Handle s_handleOcean_ShallowTint(L"Ocean_ShallowTint");
const render::Handle s_handleOcean_ReflectionTint(L"Ocean_ReflectionTint");
const render::Handle s_handleOcean_ShadowTint(L"Ocean_ShadowTint");
const render::Handle s_handleOcean_DeepColor(L"Ocean_DeepColor");
const render::Handle s_handleOcean_Opacity(L"Ocean_Opacity");
const render::Handle s_handleOcean_WavesA(L"Ocean_WavesA");
const render::Handle s_handleOcean_WavesB(L"Ocean_WavesB");
const render::Handle s_handleOcean_ReflectionTexture(L"Ocean_ReflectionTexture");

#pragma pack(1)
struct OceanVertex
{
	float pos[2];
	float edge;
};
#pragma pack()

const uint32_t c_gridSize = 512;
const uint32_t c_gridInfSize = 128;
const uint32_t c_gridCells = (c_gridSize - 1) * (c_gridSize - 1);

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.OceanComponent", OceanComponent, world::IEntityComponent)

OceanComponent::OceanComponent()
:	m_owner(0)
,	m_opacity(0.0f)
,	m_elevation(0.0f)
,	m_maxAmplitude(0.0f)
{
}

OceanComponent::~OceanComponent()
{
	destroy();
}

bool OceanComponent::create(resource::IResourceManager* resourceManager, render::IRenderSystem* renderSystem, const OceanComponentData& data)
{
	AlignedVector< render::VertexElement > vertexElements;
	vertexElements.push_back(render::VertexElement(render::DataUsage::Position, render::DtFloat2, offsetof(OceanVertex, pos)));
	vertexElements.push_back(render::VertexElement(render::DataUsage::Custom, render::DtFloat1, offsetof(OceanVertex, edge)));
	m_vertexLayout = renderSystem->createVertexLayout(vertexElements);

	m_vertexBuffer = renderSystem->createBuffer(render::BuVertex, c_gridSize * c_gridSize, sizeof(OceanVertex), false);
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

	m_indexBuffer = renderSystem->createBuffer(render::BuIndex, c_gridCells * 6, sizeof(uint32_t), false);
	if (!m_indexBuffer)
		return false;

	uint32_t* index = reinterpret_cast< uint32_t* >(m_indexBuffer->lock());
	T_ASSERT(index);

	for (uint32_t iz = 0; iz < c_gridSize - 1; ++iz)
	{
		uint32_t base = iz * c_gridSize;
		for (uint32_t ix = 0; ix < c_gridSize - 1; ++ix)
		{
			uint32_t quad[] =
			{
				base + ix,
				base + ix + 1,
				base + ix + 1 + c_gridSize,
				base + ix + c_gridSize
			};

			*index++ = quad[0];
			*index++ = quad[1];
			*index++ = quad[2];

			*index++ = quad[2];
			*index++ = quad[3];
			*index++ = quad[0];
		}
	}

	m_indexBuffer->unlock();

	m_primitives.setIndexed(render::PrimitiveType::Triangles, 0, c_gridCells * 2, 0, c_gridSize * c_gridSize - 1);

	if (!resourceManager->bind(data.m_shader, m_shader))
		return false;

	if (data.m_reflectionTexture)
	{
		if (!resourceManager->bind(data.m_reflectionTexture, m_reflectionTexture))
			return false;
	}

	m_shallowTint = data.m_shallowTint;
	m_reflectionTint = data.m_reflectionTint;
	m_shadowTint = data.m_shadowTint;
	m_deepColor = data.m_deepColor;
	m_opacity = data.m_opacity;
	m_elevation = data.m_elevation;
	m_maxAmplitude = 0.0f;

	for (int32_t i = 0; i < sizeof_array(m_wavesA); ++i)
	{
		float dx = std::cos(data.m_waves[i].direction);
		float dz = std::sin(data.m_waves[i].direction);
		m_wavesA[i] = Vector4(dx, dz, data.m_waves[i].rate, 0.0f);
		m_wavesB[i] = Vector4(data.m_waves[i].amplitude, data.m_waves[i].frequency, data.m_waves[i].phase, data.m_waves[i].pinch);
		m_maxAmplitude += std::abs(data.m_waves[i].amplitude);
	}

	return true;
}

void OceanComponent::destroy()
{
	safeDestroy(m_vertexBuffer);
	safeDestroy(m_indexBuffer);
	m_shader.clear();
}

void OceanComponent::setOwner(world::Entity* owner)
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

	bool haveTerrain = false;

	// Get terrain from owner.
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (terrainComponent)
	{
		const auto& terrain = terrainComponent->getTerrain();
		haveTerrain = (terrain && terrain->getHeightfield() && terrain->getHeightMap());
	}

	Transform transform = m_owner->getTransform() * Transform(Vector4(0.0f, m_elevation, 0.0f, 0.0f));

	const Matrix44& view = worldRenderView.getView();
	Matrix44 viewInv = view.inverse();

	// Get eye position in world space; cull entire ocean if beneath surface.
	Vector4 eye = viewInv.translation().xyz1();
	if (eye.y() < transform.translation().y() - m_maxAmplitude)
		return;

	auto perm = worldRenderPass.getPermutation(m_shader);
	m_shader->setCombination(s_handleOcean_HaveTerrain, haveTerrain, perm);
	auto sp = m_shader->getProgram(perm);
	if (!sp)
		return;

	auto renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"Ocean");
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt32;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->programParams->beginParameters(renderContext);
	renderBlock->programParams->setVectorParameter(s_handleOcean_Eye, eye);
	renderBlock->programParams->setVectorParameter(s_handleOcean_ShallowTint, m_shallowTint);
	renderBlock->programParams->setVectorParameter(s_handleOcean_ReflectionTint, m_reflectionTint);
	renderBlock->programParams->setVectorParameter(s_handleOcean_ShadowTint, m_shadowTint);
	renderBlock->programParams->setVectorParameter(s_handleOcean_DeepColor, m_deepColor);
	renderBlock->programParams->setFloatParameter(s_handleOcean_Opacity, m_opacity);
	renderBlock->programParams->setVectorArrayParameter(s_handleOcean_WavesA, m_wavesA, sizeof_array(m_wavesA));
	renderBlock->programParams->setVectorArrayParameter(s_handleOcean_WavesB, m_wavesB, sizeof_array(m_wavesB));
	renderBlock->programParams->setTextureParameter(s_handleOcean_ReflectionTexture, m_reflectionTexture);

	if (haveTerrain)
	{
		const auto& terrain = terrainComponent->getTerrain();

		const Vector4& worldExtent = terrain->getHeightfield()->getWorldExtent();
		Vector4 worldOrigin = -worldExtent * Scalar(0.5f);

		renderBlock->programParams->setTextureParameter(s_handleTerrain_Heightfield, terrain->getHeightMap());
		renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldOrigin, worldOrigin);
		renderBlock->programParams->setVectorParameter(s_handleTerrain_WorldExtent, worldExtent);
	}

	worldRenderPass.setProgramParameters(
		renderBlock->programParams,
		transform,
		transform
	);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

	}
}
