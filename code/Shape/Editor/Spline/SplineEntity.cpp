#include "Shape/Editor/Spline/ControlPointComponent.h"
#include "Shape/Editor/Spline/SplineEntity.h"
#include "Shape/Editor/Spline/SplineLayerComponent.h"
#include "World/Entity.h"
#include "World/Entity/GroupComponent.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.SplineEntity", SplineEntity, world::Entity)

SplineEntity::SplineEntity()
:	m_dirty(true)
{
}

void SplineEntity::update(const world::UpdateParams& update)
{
	auto groupComponent = getComponent< world::GroupComponent >();
	if (!groupComponent)
		return;

	// Get control points and layers.
	RefArray< ControlPointComponent > controlPoints;
	RefArray< SplineLayerComponent > layers;
	for (auto entity : groupComponent->getEntities())
	{
		auto controlPoint = entity->getComponent< ControlPointComponent >();
		if (controlPoint)
			controlPoints.push_back(controlPoint);

		auto layer = entity->getComponent< SplineLayerComponent >();
		if (layer)
			layers.push_back(layer);
	}

	// Check if any control point is dirty.
	bool controlPointsDirty = m_dirty;
	for (auto controlPoint : controlPoints)
	{
		if (controlPoint->checkDirty())
		{
			controlPointsDirty |= true;
			break;
		}
	}

	// Update transform path if any control point is dirty.
	if (controlPointsDirty)
	{
		m_path = TransformPath();
		for (uint32_t i = 0; i < controlPoints.size(); ++i)
		{
			Transform T = controlPoints[i]->getTransform();

			TransformPath::Key k;
			k.T = (float)i / (controlPoints.size() - 1);
			k.position = T.translation();
			k.orientation = T.rotation().toEulerAngles();
			k.values[0] = controlPoints[i]->getScale();
			m_path.insert(k);
		}

		for (auto layer : layers)
			layer->pathChanged(m_path);

		m_dirty = false;
	}
}

	}
}
