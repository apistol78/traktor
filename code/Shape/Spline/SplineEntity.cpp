#include "Shape/Spline/ControlPointEntity.h"
#include "Shape/Spline/ISplineLayer.h"
#include "Shape/Spline/SplineEntity.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntity", SplineEntity, world::ComponentEntity)

SplineEntity::SplineEntity(const Transform& transform)
:	world::ComponentEntity(transform)
,	m_dirty(true)
{
}

void SplineEntity::addControlPointEntity(ControlPointEntity* controlPointEntity)
{
	m_controlPointEntities.push_back(controlPointEntity);
	m_dirty = true;
}

void SplineEntity::addLayer(ISplineLayer* layer)
{
	m_layers.push_back(layer);
	m_dirty = true;
}

void SplineEntity::setTransform(const Transform& transform)
{
	Transform invTransform = getTransform().inverse();
	for (auto controlPointEntity : m_controlPointEntities)
	{
		Transform currentTransform = controlPointEntity->getTransform();
		Transform Tlocal = invTransform * currentTransform;
		Transform Tworld = transform * Tlocal;
		controlPointEntity->setTransform(Tworld);
	}
	world::ComponentEntity::setTransform(transform);
}

void SplineEntity::update(const world::UpdateParams& update)
{
	// Update control point entities, check if any is dirty.
	bool controlPointsDirty = m_dirty;
	for (auto controlPointEntity : m_controlPointEntities)
	{
		controlPointEntity->update(update);
		if (controlPointEntity->checkDirty())
			controlPointsDirty |= true;
	}

	// Update transform path if any control point is dirty.
	if (controlPointsDirty)
	{
		m_path = TransformPath();
		for (uint32_t i = 0; i < m_controlPointEntities.size(); ++i)
		{
			Transform T = m_controlPointEntities[i]->getTransform();

			TransformPath::Key k;
			k.T = (float)i / (m_controlPointEntities.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			m_path.insert(k);
		}

		for (auto layer : m_layers)
			layer->pathChanged();

		m_dirty = false;
	}

	// Update layers.
	for (auto layer : m_layers)
		layer->update(update);

	// Update spline's components.
	world::ComponentEntity::update(update);
}

void SplineEntity::build(
	world::WorldContext& worldContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	for (auto layer : m_layers)
		layer->build(worldContext, worldRenderView, worldRenderPass);
}

	}
}
