/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/AiClassFactory.h"
#include "Ai/MoveQuery.h"
#include "Ai/MoveQueryResult.h"
#include "Ai/NavMesh.h"
#include "Ai/NavMeshComponent.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

namespace traktor
{
	namespace ai
	{
		namespace
		{

Ref< BoxedVector4 > MoveQuery_update(MoveQuery* self, const Vector4& currentPosition, float nodeDistanceThreshold)
{
	Vector4 moveToPosition;
	if (self->update(currentPosition, moveToPosition, nodeDistanceThreshold))
		return new BoxedVector4(moveToPosition);
	else
		return nullptr;
}

Vector4 NavMesh_findClosestPoint(NavMesh* self, const Vector4& searchFrom)
{
	Vector4 point;
	if (self->findClosestPoint(searchFrom, point))
		return point;
	else
		return searchFrom;
}

Vector4 NavMesh_findRandomPoint_1(NavMesh* self)
{
	Vector4 point;
	if (self->findRandomPoint(point))
		return point;
	else
		return Vector4::zero();
}

Vector4 NavMesh_findRandomPoint_2(NavMesh* self, const Vector4& center, float radius)
{
	Vector4 point;
	if (self->findRandomPoint(center, radius, point))
		return point;
	else
		return Vector4::zero();
}

const NavMesh* NavMeshComponent_get(NavMeshComponent* self)
{
	return self->get();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.AiClassFactory", 0, AiClassFactory, IRuntimeClassFactory)

void AiClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classMoveQuery = new AutoRuntimeClass< MoveQuery >();
	classMoveQuery->addMethod("update", &MoveQuery_update);
	registrar->registerClass(classMoveQuery);

	auto classMoveQueryResult = new AutoRuntimeClass< MoveQueryResult >();
	classMoveQueryResult->addMethod("get", &MoveQueryResult::get);
	registrar->registerClass(classMoveQueryResult);

	auto classNavMesh = new AutoRuntimeClass< NavMesh >();
	classNavMesh->addMethod("createMoveQuery", &NavMesh::createMoveQuery);
	classNavMesh->addMethod("findClosestPoint", &NavMesh_findClosestPoint);
	classNavMesh->addMethod("findRandomPoint", &NavMesh_findRandomPoint_1);
	classNavMesh->addMethod("findRandomPoint", &NavMesh_findRandomPoint_2);
	registrar->registerClass(classNavMesh);

	auto classNavMeshComponent = new AutoRuntimeClass< NavMeshComponent >();
	classNavMeshComponent->addMethod("get", &NavMeshComponent_get);
	registrar->registerClass(classNavMeshComponent);
}

	}
}
