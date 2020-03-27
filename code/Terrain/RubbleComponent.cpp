#include <limits>
#include "Core/Containers/StaticVector.h"
#include "Core/Log/Log.h"
#include "Core/Math/Half.h"
#include "Core/Math/RandomGeometry.h"
#include "Heightfield/Heightfield.h"
#include "Render/ISimpleTexture.h"
#include "Render/Context/RenderContext.h"
#include "Resource/IResourceManager.h"
#include "Terrain/Terrain.h"
#include "Terrain/TerrainComponent.h"
#include "Terrain/TerrainSurfaceCache.h"
#include "Terrain/RubbleComponent.h"
#include "Terrain/RubbleComponentData.h"
#include "World/Entity.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

const render::Handle s_handleNormals(L"Normals");
const render::Handle s_handleHeightfield(L"Heightfield");
const render::Handle s_handleSurface(L"Surface");
const render::Handle s_handleWorldExtent(L"WorldExtent");
const render::Handle s_handleEye(L"Eye");
const render::Handle s_handleMaxDistance(L"MaxDistance");

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.RubbleComponent", RubbleComponent, TerrainLayerComponent)

RubbleComponent::RubbleComponent()
:	m_owner(nullptr)
,	m_spreadDistance(0.0f)
,	m_clusterSize(0.0f)
,	m_eye(Vector4::zero())
{
}

bool RubbleComponent::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const RubbleComponentData& layerData
)
{
	m_spreadDistance = layerData.m_spreadDistance;

	m_rubble.resize(layerData.m_rubble.size());
	for (size_t i = 0; i < m_rubble.size(); ++i)
	{
		if (!resourceManager->bind(layerData.m_rubble[i].mesh, m_rubble[i].mesh))
			return false;

		m_rubble[i].material = layerData.m_rubble[i].material;
		m_rubble[i].density = layerData.m_rubble[i].density;
		m_rubble[i].randomScaleAmount = layerData.m_rubble[i].randomScaleAmount;
	}

	return true;
}

void RubbleComponent::destroy()
{
}

void RubbleComponent::setOwner(world::Entity* owner)
{
	TerrainLayerComponent::setOwner(owner);
	m_owner = owner;
}

void RubbleComponent::setTransform(const Transform& transform)
{
}

Aabb3 RubbleComponent::getBoundingBox() const
{
	return Aabb3();
}

void RubbleComponent::update(const world::UpdateParams& update)
{
	TerrainLayerComponent::update(update);
}

void RubbleComponent::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();

	// Update clusters at first pass from eye pow.
	bool updateClusters = bool((worldRenderPass.getPassFlags() & world::IWorldRenderPass::PfFirst) != 0);

	Matrix44 view = worldRenderView.getView();
	Vector4 eye = view.inverse().translation();

	if (updateClusters)
	{
		Frustum viewFrustum = worldRenderView.getViewFrustum();
		viewFrustum.setFarZ(Scalar(m_spreadDistance + m_clusterSize));

		// Only perform "replanting" when moved more than one unit.
		if ((eye - m_eye).length() >= m_clusterSize / 2.0f)
		{
			m_eye = eye;
			for (auto& cluster : m_clusters)
			{
				cluster.distance = (cluster.center - eye).length();

				bool visible = cluster.visible;
				cluster.visible = (bool)(viewFrustum.inside(view * cluster.center, Scalar(m_clusterSize)) != Frustum::IrOutside);
				if (!cluster.visible)
					continue;
				if (cluster.visible && visible)
					continue;

				const float randomScaleAmount = cluster.rubbleDef->randomScaleAmount;

				// Use cluster center as random seed.
				RandomGeometry random(
					(int32_t)(cluster.center.x() * 919.0f + cluster.center.z() * 463.0f)
				);
				for (int32_t j = cluster.from; j < cluster.to; ++j)
				{
					float dx = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;
					float dz = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;

					float px = cluster.center.x() + dx;
					float pz = cluster.center.z() + dz;
					float py = terrain->getHeightfield()->getWorldHeight(px, pz);

					m_instances[j].position = Vector4(px, py, pz, 0.0f);
					m_instances[j].rotation = Quaternion::fromEulerAngles(random.nextFloat() * TWO_PI, (random.nextFloat() * 2.0f - 1.0f) * deg2rad(10.0f), 0.0f);
					m_instances[j].scale = random.nextFloat() * randomScaleAmount + (1.0f - randomScaleAmount);
				}
			}
		}
	}

	render::RenderContext* renderContext = context.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleNormals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleSurface, terrainComponent->getSurfaceCache(worldRenderView.getIndex())->getBaseTexture());
	extraParameters->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
	extraParameters->setVectorParameter(s_handleEye, eye);
	extraParameters->setFloatParameter(s_handleMaxDistance, m_spreadDistance + m_clusterSize);
	extraParameters->endParameters(renderContext);

	for (const auto& cluster : m_clusters)
	{
		if (!cluster.visible)
			continue;

		int32_t count = cluster.to - cluster.from;
		for (int32_t j = 0; j < count; )
		{
			int32_t batch = std::min< int32_t >(count - j, mesh::InstanceMesh::MaxInstanceCount);

			m_instanceData.resize(batch);
			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				m_instances[j + cluster.from].position.storeAligned( m_instanceData[k].data.translation );
				m_instances[j + cluster.from].rotation.e.storeAligned( m_instanceData[k].data.rotation );
				m_instanceData[k].data.scale = m_instances[j + cluster.from].scale;
				m_instanceData[k].distance = cluster.distance;
			}

			cluster.rubbleDef->mesh->build(
				renderContext,
				worldRenderPass,
				m_instanceData,
				extraParameters
			);
		}
	}
}

void RubbleComponent::updatePatches()
{
	m_instances.resize(0);
	m_clusters.resize(0);

	auto terrainComponent = m_owner->getComponent< TerrainComponent >();
	if (!terrainComponent)
		return;

	const auto& terrain = terrainComponent->getTerrain();
	const auto& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um(16, 0);
	uint8_t maxMaterialIndex = 0;
	for (const auto& rubble : m_rubble)
		um[rubble.material] = ++maxMaterialIndex;

	int32_t size = heightfield->getSize();
	Vector4 extentPerGrid = heightfield->getWorldExtent() / Scalar(float(size));

	m_clusterSize = (16.0f / 2.0f) * max< float >(extentPerGrid.x(), extentPerGrid.z());

	// Create clusters.
	RandomGeometry random;
	for (int32_t z = 0; z < size; z += 16)
	{
		for (int32_t x = 0; x < size; x += 16)
		{
			StaticVector< int32_t, 16 > cm(16, 0);
			int32_t totalDensity = 0;
			for (int32_t cz = 0; cz < 16; ++cz)
			{
				for (int32_t cx = 0; cx < 16; ++cx)
				{
					uint8_t material = heightfield->getGridMaterial(x + cx, z + cz);
					uint8_t index = um[material];
					if (index > 0)
					{
						cm[index - 1]++;
						totalDensity++;
					}
				}
			}
			if (totalDensity <= 0)
				continue;

			float wx, wz;
			heightfield->gridToWorld(x + 8, z + 8, wx, wz);
			float wy = heightfield->getWorldHeight(wx, wz);

			for (uint32_t i = 0; i < maxMaterialIndex; ++i)
			{
				if (cm[i] <= 0)
					continue;

				for (auto& rubble : m_rubble)
				{
					if (um[rubble.material] != i + 1)
						continue;

					int32_t densityFactor = cm[i];
					int32_t density = (rubble.density * densityFactor) / (16 * 16);
					if (density <= 4)
						continue;

					int32_t from = int32_t(m_instances.size());
					for (int32_t k = 0; k < density; ++k)
					{
						Instance instance;
						instance.position = Vector4::zero();
						instance.rotation = Quaternion::identity();
						instance.scale = 0.0f;
						m_instances.push_back(instance);
					}
					int32_t to = int32_t(m_instances.size());

					Cluster& c = m_clusters.push_back();
					c.rubbleDef = &rubble;
					c.center = Vector4(wx, wy, wz, 1.0f);
					c.distance = std::numeric_limits< float >::max();
					c.visible = false;
					c.from = from;
					c.to = to;
				}
			}
		}
	}
}

	}
}
