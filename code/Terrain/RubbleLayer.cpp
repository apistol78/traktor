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
#include "Terrain/RubbleLayer.h"
#include "Terrain/RubbleLayerData.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"

namespace traktor
{
	namespace terrain
	{
		namespace
		{

render::handle_t s_handleNormals;
render::handle_t s_handleHeightfield;
render::handle_t s_handleSurface;
render::handle_t s_handleWorldExtent;
render::handle_t s_handleEye;
render::handle_t s_handleMaxDistance;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.terrain.RubbleLayer", RubbleLayer, ITerrainLayer)

RubbleLayer::RubbleLayer()
:	m_spreadDistance(0.0f)
,	m_clusterSize(0.0f)
,	m_eye(Vector4::zero())
{
	s_handleNormals = render::getParameterHandle(L"Normals");
	s_handleHeightfield = render::getParameterHandle(L"Heightfield");
	s_handleSurface = render::getParameterHandle(L"Surface");
	s_handleWorldExtent = render::getParameterHandle(L"WorldExtent");
	s_handleEye = render::getParameterHandle(L"Eye");
	s_handleMaxDistance = render::getParameterHandle(L"MaxDistance");
}

bool RubbleLayer::create(
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	const RubbleLayerData& layerData,
	const TerrainComponent& terrainComponent
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

	updatePatches(terrainComponent);
	return true;
}

void RubbleLayer::update(const world::UpdateParams& update)
{
}

void RubbleLayer::render(
	TerrainComponent& terrainComponent,
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
	const resource::Proxy< Terrain >& terrain = terrainComponent.getTerrain();

	// Update clusters at first pass from eye pow.
	bool updateClusters = worldRenderPass.isFirstPassFromEye();

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

			for (AlignedVector< Cluster >::iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
			{
				i->distance = (i->center - eye).length();

				bool visible = i->visible;

				i->visible = (viewFrustum.inside(view * i->center, Scalar(m_clusterSize)) != Frustum::IrOutside);
				if (!i->visible)
					continue;

				if (i->visible && visible)
					continue;

				RandomGeometry random(int32_t(i->center.x() * 919.0f + i->center.z() * 463.0f));
				for (int32_t j = i->from; j < i->to; ++j)
				{
					float dx = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;
					float dz = (random.nextFloat() * 2.0f - 1.0f) * m_clusterSize;

					float px = i->center.x() + dx;
					float pz = i->center.z() + dz;

					float py = terrain->getHeightfield()->getWorldHeight(px, pz);

					m_instances[j].position = Vector4(px, py, pz, 0.0f);
					m_instances[j].rotation = Quaternion::fromEulerAngles(random.nextFloat() * TWO_PI, (random.nextFloat() * 2.0f - 1.0f) * deg2rad(10.0f), 0.0f);
					m_instances[j].scale = random.nextFloat() * i->scale + (1.0f - i->scale);
				}
			}
		}
	}

	render::RenderContext* renderContext = worldContext.getRenderContext();

	// Expose some more shader parameters, such as terrain color etc.
	render::ProgramParameters* extraParameters = renderContext->alloc< render::ProgramParameters >();
	extraParameters->beginParameters(renderContext);
	extraParameters->setTextureParameter(s_handleNormals, terrain->getNormalMap());
	extraParameters->setTextureParameter(s_handleHeightfield, terrain->getHeightMap());
	extraParameters->setTextureParameter(s_handleSurface, terrainComponent.getSurfaceCache()->getBaseTexture());
	extraParameters->setVectorParameter(s_handleWorldExtent, terrain->getHeightfield()->getWorldExtent());
	extraParameters->setVectorParameter(s_handleEye, eye);
	extraParameters->setFloatParameter(s_handleMaxDistance, m_spreadDistance + m_clusterSize);
	extraParameters->endParameters(renderContext);

	for (AlignedVector< Cluster >::const_iterator i = m_clusters.begin(); i != m_clusters.end(); ++i)
	{
		if (!i->visible)
			continue;

		int32_t count = i->to - i->from;
		for (int32_t j = 0; j < count; )
		{
			int32_t batch = std::min< int32_t >(count - j, mesh::InstanceMesh::MaxInstanceCount);

			m_instanceData.resize(batch);
			for (int32_t k = 0; k < batch; ++k, ++j)
			{
				m_instances[j + i->from].position.storeAligned( m_instanceData[k].first.translation );
				m_instances[j + i->from].rotation.e.storeAligned( m_instanceData[k].first.rotation );
				m_instanceData[k].first.scale = m_instances[j + i->from].scale;
				m_instanceData[k].second = i->distance;
			}

			i->rubbleDef->mesh->render(
				renderContext,
				worldRenderPass,
				m_instanceData,
				extraParameters
			);
		}
	}
}

void RubbleLayer::updatePatches(const TerrainComponent& terrainComponent)
{
	m_instances.resize(0);
	m_clusters.resize(0);

	const resource::Proxy< Terrain >& terrain = terrainComponent.getTerrain();
	const resource::Proxy< hf::Heightfield >& heightfield = terrain->getHeightfield();

	// Get set of materials which have undergrowth.
	StaticVector< uint8_t, 16 > um(16, 0);
	uint8_t maxMaterialIndex = 0;
	for (std::vector< RubbleMesh >::const_iterator i = m_rubble.begin(); i != m_rubble.end(); ++i)
		um[i->material] = ++maxMaterialIndex;

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

				for (std::vector< RubbleMesh >::iterator j = m_rubble.begin(); j != m_rubble.end(); ++j)
				{
					if (um[j->material] == i + 1)
					{
						int32_t densityFactor = cm[i];

						int32_t density = (j->density * densityFactor) / (16 * 16);
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

						Cluster c;
						c.rubbleDef = &(*j);
						c.center = Vector4(wx, wy, wz, 1.0f);
						c.distance = std::numeric_limits< float >::max();
						c.visible = false;
						c.from = from;
						c.to = to;
						m_clusters.push_back(c);
					}
				}
			}
		}
	}
}

	}
}
