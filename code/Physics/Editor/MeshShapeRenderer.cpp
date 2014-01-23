#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/Editor/MeshShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"

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
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform& body1Transform0,
	const Transform& body1Transform,
	const ShapeDesc* shapeDesc
) const
{
	const MeshShapeDesc* meshShapeDesc = checked_type_cast< const MeshShapeDesc*, false >(shapeDesc);

	resource::Proxy< Mesh > mesh;
	if (resourceManager->bind(meshShapeDesc->getMesh(), mesh))
	{
		Transform T = body1Transform * Transform(mesh->getOffset()) * shapeDesc->getLocalTransform();

		Vector4 Vc = primitiveRenderer->getView() * T.translation().xyz1();
		if (Vc.z() < 0.0f || Vc.z() > 100.0f)
			return;

		primitiveRenderer->pushWorld(T.toMatrix44());

		const AlignedVector< Vector4 >& vertices = mesh->getVertices();

		const std::vector< Mesh::Triangle >& shapeTriangles = mesh->getShapeTriangles();
		const std::vector< Mesh::Triangle >& hullTriangles = mesh->getHullTriangles();

		if (hullTriangles.empty())
		{
			Scalar margin(mesh->getMargin());
			for (std::vector< Mesh::Triangle >::const_iterator i = shapeTriangles.begin(); i != shapeTriangles.end(); ++i)
			{
				const Vector4& V0 = vertices[i->indices[0]];
				const Vector4& V1 = vertices[i->indices[1]];
				const Vector4& V2 = vertices[i->indices[2]];

				Vector4 N = cross(V0 - V1, V2 - V1).normalized();
				Vector4 offset = N * margin;

				primitiveRenderer->drawSolidTriangle(V0 + offset, V1 + offset, V2 + offset, Color4ub(128, 255, 255, 128));
				primitiveRenderer->drawWireTriangle(V0 + offset, V1 + offset, V2 + offset, Color4ub(0, 255, 255, 180));
			}
		}
		else
		{
			Scalar margin(mesh->getMargin());
			for (std::vector< Mesh::Triangle >::const_iterator i = hullTriangles.begin(); i != hullTriangles.end(); ++i)
			{
				const Vector4& V0 = vertices[i->indices[0]];
				const Vector4& V1 = vertices[i->indices[1]];
				const Vector4& V2 = vertices[i->indices[2]];

				Vector4 N = cross(V0 - V1, V2 - V1).normalized();
				Vector4 offset = N * margin;

				primitiveRenderer->drawSolidTriangle(V0 + offset, V1 + offset, V2 + offset, Color4ub(128, 255, 255, 128));
				primitiveRenderer->drawWireTriangle(V0 + offset, V1 + offset, V2 + offset, Color4ub(0, 255, 255, 180));
			}
		}

		primitiveRenderer->popWorld();
	}
}

	}
}
