#include "Shape/Editor/Spline/ExtrudeShapeLayer.h"
#include "Shape/Editor/Spline/ExtrudeShapeLayerData.h"

namespace traktor
{
	namespace shape
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.shape.ExtrudeShapeLayer", ExtrudeShapeLayer, SplineLayerComponent)

ExtrudeShapeLayer::ExtrudeShapeLayer(const ExtrudeShapeLayerData* data)
:	m_data(data)
{
}

void ExtrudeShapeLayer::destroy()
{
}

void ExtrudeShapeLayer::setOwner(world::Entity* owner)
{
}

void ExtrudeShapeLayer::setTransform(const Transform& transform)
{
}

Aabb3 ExtrudeShapeLayer::getBoundingBox() const
{
	return Aabb3();
}

void ExtrudeShapeLayer::update(const world::UpdateParams& update)
{
}

void ExtrudeShapeLayer::pathChanged(const TransformPath& path)
{
}

	}
}
