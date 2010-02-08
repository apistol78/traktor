#include "Mesh/Stream/StreamMeshEntity.h"
#include "World/WorldContext.h"
#include "World/WorldRenderView.h"
#include "World/Entity/EntityUpdate.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.StreamMeshEntity", StreamMeshEntity, MeshEntity)

StreamMeshEntity::StreamMeshEntity(const Transform& transform, const resource::Proxy< StreamMesh >& mesh)
:	MeshEntity(transform)
,	m_mesh(mesh)
,	m_frame(0)
{
}

uint32_t StreamMeshEntity::getFrameCount() const
{
	if (!validate())
		return 0;

	return m_mesh->getFrameCount();
}

void StreamMeshEntity::setFrame(uint32_t frame)
{
	m_frame = frame;
}

Aabb StreamMeshEntity::getBoundingBox() const
{
	return validate() ? m_mesh->getBoundingBox() : Aabb();
}

void StreamMeshEntity::render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance)
{
	if (!validate())
		return;

	if (m_frame >= m_mesh->getFrameCount())
		return;

	m_mesh->render(
		worldContext->getRenderContext(),
		worldRenderView,
		m_transform,
		m_transformPrevious,
		m_instance,
		m_frame,
		distance,
		getParameterCallback()
	);

	m_transformPrevious = m_transform;
}

void StreamMeshEntity::update(const world::EntityUpdate* update)
{
	if (!validate())
		return;

	static float s_time = 0.0f; s_time += update->getDeltaTime();
	m_frame = uint32_t(s_time * 30.0f) % m_mesh->getFrameCount();
}

bool StreamMeshEntity::validate() const
{
	if (m_mesh.valid() && m_instance)
		return true;

	if (!m_mesh.validate())
		return false;

	m_instance = m_mesh->createInstance();
	if (!m_instance)
		return false;

	return true;
}

	}
}
