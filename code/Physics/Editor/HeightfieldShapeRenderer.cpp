#include "Heightfield/Heightfield.h"
#include "Physics/HeightfieldShapeDesc.h"
#include "Physics/Editor/HeightfieldShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.HeightfieldShapeRenderer", 0, HeightfieldShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& HeightfieldShapeRenderer::getDescType() const
{
	return type_of< HeightfieldShapeDesc >();
}

void HeightfieldShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	const HeightfieldShapeDesc* heightfieldShapeDesc = checked_type_cast< const HeightfieldShapeDesc*, false >(shapeDesc);

	resource::Proxy< hf::Heightfield > heightfield = heightfieldShapeDesc->getHeightfield();
	if (resourceManager->bind(heightfield) && heightfield.validate())
	{
		const Vector4& extent = heightfield->getResource().getWorldExtent();
		Aabb3 boundingBox(-extent / Scalar(2.0f), extent / Scalar(2.0f));
		primitiveRenderer->drawWireAabb(boundingBox, Color4ub(0, 255, 255, 180));
	}
}

	}
}
