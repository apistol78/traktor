#include "Physics/MeshShapeDesc.h"
#include "Physics/Editor/MeshShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshShapeRenderer", 0, MeshShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& MeshShapeRenderer::getDescType() const
{
	return type_of< MeshShapeDesc >();
}

void MeshShapeRenderer::draw(
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	const MeshShapeDesc* meshShapeDesc = checked_type_cast< const MeshShapeDesc*, false >(shapeDesc);

	//resource::Proxy< Mesh > mesh = meshShapeDesc->getMesh();
	//if (getContext()->getResourceManager()->bind(mesh) && mesh.validate())
	//{
	//	const AlignedVector< Vector4 >& vertices = mesh->getVertices();
	//	const std::vector< Mesh::Triangle >& triangles = 
	//		m_showHull ?
	//		mesh->getHullTriangles() :
	//	mesh->getShapeTriangles();

	//	for (std::vector< Mesh::Triangle >::const_iterator i = triangles.begin(); i != triangles.end(); ++i)
	//	{
	//		const Vector4& V0 = vertices[i->indices[0]];
	//		const Vector4& V1 = vertices[i->indices[1]];
	//		const Vector4& V2 = vertices[i->indices[2]];

	//		if (getEntityAdapter()->isSelected())
	//		{
	//			primitiveRenderer->drawSolidTriangle(V0, V1, V2, Color4ub(128, 255, 255, 128));
	//			primitiveRenderer->drawWireTriangle(V0, V1, V2, Color4ub(0, 255, 255));
	//		}
	//		else
	//			primitiveRenderer->drawWireTriangle(V0, V1, V2, Color4ub(0, 255, 255, 180));
	//	}
	//}
}

	}
}
