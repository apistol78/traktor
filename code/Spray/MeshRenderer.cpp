#include "Spray/MeshRenderer.h"
#include "World/IWorldRenderPass.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

const uint32_t c_maxInstances = 16;

		}

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
	if (p.size() <= c_maxInstances)
	{
		p.insert(
			p.end(),
			points.begin(),
			points.end()
		);
		m_meshes[mesh].second = meshOrientationFromVelocity;
	}
}

void MeshRenderer::flush(
	render::RenderContext* renderContext,
	world::IWorldRenderPass& worldRenderPass
)
{
	for (std::map< Ref< mesh::InstanceMesh >, std::pair< PointVector, bool > >::iterator i = m_meshes.begin(); i != m_meshes.end(); ++i)
	{
		if (i->second.first.empty())
			continue;

		if (!i->first->supportTechnique(worldRenderPass.getTechnique()))
		{
			i->second.first.resize(0);
			continue;
		}

		m_instances.resize(i->second.first.size());
		for (uint32_t j = 0; j < i->second.first.size(); ++j)
		{
			Quaternion R =
				i->second.second ? Quaternion::fromAxisAngle(
					cross(i->second.first[j].velocity, Vector4(0.0f, 1.0f, 0.0f)).normalized(),
					i->second.first[j].orientation
				) :
				Quaternion::identity();

			R.e.storeUnaligned(m_instances[j].first.rotation);
			i->second.first[j].position.storeUnaligned(m_instances[j].first.translation);
			m_instances[j].first.scale = i->second.first[j].size;
			m_instances[j].second = 0.0f;
		}

		i->first->render(
			renderContext,
			worldRenderPass,
			m_instances
		);

		i->second.first.resize(0);
	}
}

	}
}
