#include "Mesh/Instance/InstanceMesh.h"
#include "Spray/MeshRenderer.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.MeshRenderer", MeshRenderer, Object)

MeshRenderer::MeshRenderer()
{
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::destroy()
{
}

void MeshRenderer::render(
	mesh::InstanceMesh* mesh,
	bool meshOrientationFromVelocity,
	const PointVector& points
)
{
	PointVector& p = m_meshes[mesh].first;
	p.insert(
		p.end(),
		points.begin(),
		points.end()
	);
	m_meshes[mesh].second = meshOrientationFromVelocity;
}

void MeshRenderer::flush(
	render::RenderContext* renderContext,
	world::IWorldRenderPass& worldRenderPass
)
{
	AlignedVector< mesh::InstanceMesh::instance_distance_t > instances;
	for (SmallMap< Ref< mesh::InstanceMesh >, std::pair< PointVector, bool > >::const_iterator i = m_meshes.begin(); i != m_meshes.end(); ++i)
	{
		if (!i->first->supportTechnique(worldRenderPass.getTechnique()))
			continue;

		instances.resize(i->second.first.size());
		for (uint32_t j = 0; j < i->second.first.size(); ++j)
		{
			Quaternion R =
				i->second.second ? Quaternion::fromAxisAngle(
					cross(i->second.first[j].velocity, Vector4(0.0f, 1.0f, 0.0f)).normalized(),
					i->second.first[j].orientation
				) :
				Quaternion::identity();

			R.e.storeUnaligned(instances[j].first.rotation);
			i->second.first[j].position.storeUnaligned(instances[j].first.translation);
			instances[j].first.scale = i->second.first[j].size;
			instances[j].second = 0.0f;
		}

		i->first->render(
			renderContext,
			worldRenderPass,
			instances
		);
	}
	m_meshes.clear();
}

	}
}
