#include "Physics/JointDesc.h"
#include "Physics/ShapeDesc.h"
#include "Physics/Editor/IPhysicsJointRenderer.h"
#include "Physics/Editor/IPhysicsShapeRenderer.h"
#include "Physics/Editor/PhysicsRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.physics.PhysicsRenderer", PhysicsRenderer, Object)

PhysicsRenderer::PhysicsRenderer()
{
	TypeInfoSet jointRendererTypes;
	type_of< IPhysicsJointRenderer >().findAllOf(jointRendererTypes, false);

	for (TypeInfoSet::const_iterator i = jointRendererTypes.begin(); i != jointRendererTypes.end(); ++i)
	{
		Ref< IPhysicsJointRenderer > jointRenderer = checked_type_cast< IPhysicsJointRenderer*, false >((*i)->createInstance());
		m_jointRenderers.insert(std::make_pair(&jointRenderer->getDescType(), jointRenderer));
	}

	TypeInfoSet shapeRendererTypes;
	type_of< IPhysicsShapeRenderer >().findAllOf(shapeRendererTypes, false);

	for (TypeInfoSet::const_iterator i = shapeRendererTypes.begin(); i != shapeRendererTypes.end(); ++i)
	{
		Ref< IPhysicsShapeRenderer > shapeRenderer = checked_type_cast< IPhysicsShapeRenderer*, false >((*i)->createInstance());
		m_shapeRenderers.insert(std::make_pair(&shapeRenderer->getDescType(), shapeRenderer));
	}
}

void PhysicsRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const JointDesc* jointDesc
) const
{
	std::map< const TypeInfo*, Ref< IPhysicsJointRenderer > >::const_iterator i = m_jointRenderers.find(&type_of(jointDesc));
	if (i != m_jointRenderers.end())
		i->second->draw(
			primitiveRenderer,
			body1Transform0,
			body1Transform,
			jointDesc
		);
}

void PhysicsRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const Transform& body2Transform0,
	const Transform& body2Transform,
	const JointDesc* jointDesc
) const
{
	std::map< const TypeInfo*, Ref< IPhysicsJointRenderer > >::const_iterator i = m_jointRenderers.find(&type_of(jointDesc));
	if (i != m_jointRenderers.end())
		i->second->draw(
			primitiveRenderer,
			body1Transform0,
			body1Transform,
			body2Transform0,
			body2Transform,
			jointDesc
		);
}

void PhysicsRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	std::map< const TypeInfo*, Ref< IPhysicsShapeRenderer > >::const_iterator i = m_shapeRenderers.find(&type_of(shapeDesc));
	if (i != m_shapeRenderers.end())
		i->second->draw(
			resourceManager,
			primitiveRenderer,
			body1Transform0,
			body1Transform,
			shapeDesc
		);
}

	}
}
