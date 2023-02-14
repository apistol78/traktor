/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/Mesh.h"
#include "Physics/MeshShapeDesc.h"
#include "Physics/Editor/MeshShapeRenderer.h"
#include "Render/PrimitiveRenderer.h"
#include "Resource/IResourceManager.h"

namespace traktor::physics
{
	namespace
	{

const Color4ub c_faceColors[] =
{
	Color4ub(128, 255, 255, 128),
	Color4ub(255, 128, 255, 128),
	Color4ub(255, 255, 128, 128)
};

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.MeshShapeRenderer", 0, MeshShapeRenderer, IPhysicsShapeRenderer)

const TypeInfo& MeshShapeRenderer::getDescType() const
{
	return type_of< MeshShapeDesc >();
}

void MeshShapeRenderer::draw(
	resource::IResourceManager* resourceManager,
	render::PrimitiveRenderer* primitiveRenderer,
	const Transform body1Transform[2],
	const ShapeDesc* shapeDesc
) const
{
	const MeshShapeDesc* meshShapeDesc = mandatory_non_null_type_cast< const MeshShapeDesc* >(shapeDesc);

	resource::Proxy< Mesh > mesh;
	if (resourceManager->bind(meshShapeDesc->getMesh(), mesh))
	{
		Transform T = body1Transform[1] * Transform(mesh->getOffset()) * shapeDesc->getLocalTransform();

		Vector4 Vc = primitiveRenderer->getView() * T.translation().xyz1();
		if (Vc.z() < 0.0f || Vc.z() > 100.0f)
			return;

		primitiveRenderer->pushWorld(T.toMatrix44());

		Vector4 eyePosition = (primitiveRenderer->getView() * primitiveRenderer->getWorld()).inverse().translation().xyz1();

		const auto& vertices = mesh->getVertices();
		const auto& shapeTriangles = mesh->getShapeTriangles();
		const auto& hullTriangles = mesh->getHullTriangles();

		if (hullTriangles.empty())
		{
			Scalar margin(mesh->getMargin());
			for (const auto& triangle : shapeTriangles)
			{
				const Vector4& V0 = vertices[triangle.indices[0]];
				const Vector4& V1 = vertices[triangle.indices[1]];
				const Vector4& V2 = vertices[triangle.indices[2]];

				Vector4 N = cross(V0 - V1, V2 - V1).normalized();
				if (dot3(eyePosition - V0, N) >= 0.0f)
				{
					Vector4 offset = N * margin;
					primitiveRenderer->drawSolidTriangle(V0 + offset, V1 + offset, V2 + offset, c_faceColors[triangle.material % sizeof_array(c_faceColors)]);
					primitiveRenderer->drawWireTriangle(V0 + offset, V1 + offset, V2 + offset, Color4ub(0, 255, 255, 180));
				}
			}
		}
		else
		{
			Scalar margin(mesh->getMargin());
			for (const auto& triangle : shapeTriangles)
			{
				const Vector4& V0 = vertices[triangle.indices[0]];
				const Vector4& V1 = vertices[triangle.indices[1]];
				const Vector4& V2 = vertices[triangle.indices[2]];

				Vector4 N = cross(V0 - V1, V2 - V1).normalized();
				if (dot3(eyePosition - V0, N) >= 0.0f)
				{
					Vector4 offset = N * margin;
					primitiveRenderer->drawSolidTriangle(V0 + offset, V1 + offset, V2 + offset, c_faceColors[triangle.material % sizeof_array(c_faceColors)]);
					primitiveRenderer->drawWireTriangle(V0 + offset, V1 + offset, V2 + offset, Color4ub(0, 255, 255, 180));
				}
			}
		}

		primitiveRenderer->popWorld();
	}
}

}
