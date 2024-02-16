/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/MeshRenderer.h"
#include "World/IWorldRenderPass.h"

namespace traktor::spray
{
	namespace
	{

const uint32_t c_maxInstances = 16;

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.MeshRenderer", MeshRenderer, Object)

void MeshRenderer::render(
	mesh::InstanceMesh* mesh,
	bool meshOrientationFromVelocity,
	const pointVector_t& points
)
{
	pointVector_t& p = m_meshes[mesh].first;
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
	const world::IWorldRenderPass& worldRenderPass
)
{
/*
	for (SmallMap< Ref< mesh::InstanceMesh >, std::pair< pointVector_t, bool > >::iterator i = m_meshes.begin(); i != m_meshes.end(); ++i)
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
			const Quaternion R =
				i->second.second ? Quaternion::fromAxisAngle(
					cross(i->second.first[j].velocity, Vector4(0.0f, 1.0f, 0.0f)).normalized(),
					i->second.first[j].orientation
				) :
				Quaternion::identity();

			R.e.storeUnaligned(m_instances[j].data.rotation);
			i->second.first[j].position.storeUnaligned(m_instances[j].data.translation);
			m_instances[j].data.scale = i->second.first[j].size;
			m_instances[j].distance = 0.0f;
		}

		i->first->build(
			renderContext,
			worldRenderPass,
			m_instances,
			nullptr
		);

		i->second.first.resize(0);
	}
*/
}

}
