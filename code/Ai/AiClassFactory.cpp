/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ai/AiClassFactory.h"
#include "Ai/MoveQuery.h"
#include "Ai/MoveQueryResult.h"
#include "Ai/NavMeshComponent.h"
#include "Ai/NavMeshModifierEntityComponent.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes/BoxedVector4.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

namespace traktor::ai
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

Ref< BoxedVector4 > MoveQuery_constrain(MoveQuery* self, const Vector4& currentPosition, const Vector4& desiredPosition)
{
	Vector4 position;
	if (self->constrain(currentPosition, desiredPosition, position))
		return new BoxedVector4(position);
	else
		return nullptr;
}

Vector4 NavMeshComponent_findClosestPoint(NavMeshComponent* self, const Vector4& searchFrom, float searchDistance)
{
	Vector4 point;
	if (self->findClosestPoint(searchFrom, searchDistance, point))
		return point;
	else
		return searchFrom;
}

Vector4 NavMeshComponent_findClosestPointXZ(NavMeshComponent* self, const Vector4& searchFrom, float searchDistance)
{
	Vector4 point;
	if (self->findClosestPointXZ(searchFrom, searchDistance, point))
		return point;
	else
		return searchFrom;
}

Vector4 NavMeshComponent_findRandomPoint_1(NavMeshComponent* self)
{
	Vector4 point;
	if (self->findRandomPoint(point))
		return point;
	else
		return Vector4::zero();
}

Vector4 NavMeshComponent_findRandomPoint_2(NavMeshComponent* self, const Vector4& center, float radius)
{
	Vector4 point;
	if (self->findRandomPoint(center, radius, point))
		return point;
	else
		return Vector4::zero();
}

Any NavMeshComponent_findDistanceToWall_1(NavMeshComponent* self, const Vector4& center, float radius)
{
	float distance;
	if (self->findDistanceToWall(center, radius, distance))
		return Any::fromFloat(distance);
	else
		return Any();
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.AiClassFactory", 0, AiClassFactory, IRuntimeClassFactory)

void AiClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classMoveQuery = new AutoRuntimeClass< MoveQuery >();
	classMoveQuery->addMethod("update", &MoveQuery_update);
	classMoveQuery->addMethod("constrain", &MoveQuery_constrain);
	classMoveQuery->addMethod("getSteerPathCount", &MoveQuery::getSteerPathCount);
	classMoveQuery->addMethod("getSteerPathPosition", &MoveQuery::getSteerPathPosition);
	classMoveQuery->addMethod("getSteerIndex", &MoveQuery::getSteerIndex);
	registrar->registerClass(classMoveQuery);

	auto classMoveQueryResult = new AutoRuntimeClass< MoveQueryResult >();
	classMoveQueryResult->addMethod("get", &MoveQueryResult::get);
	registrar->registerClass(classMoveQueryResult);

	auto classNavMeshComponent = new AutoRuntimeClass< NavMeshComponent >();
	classNavMeshComponent->addMethod("createMoveQuery", &NavMeshComponent::createMoveQuery);
	classNavMeshComponent->addMethod("findClosestPoint", &NavMeshComponent_findClosestPoint);
	classNavMeshComponent->addMethod("findClosestPointXZ", &NavMeshComponent_findClosestPointXZ);
	classNavMeshComponent->addMethod("findRandomPoint", &NavMeshComponent_findRandomPoint_1);
	classNavMeshComponent->addMethod("findRandomPoint", &NavMeshComponent_findRandomPoint_2);
	classNavMeshComponent->addMethod("findDistanceToWall", &NavMeshComponent_findDistanceToWall_1);
	registrar->registerClass(classNavMeshComponent);

	auto classNavMeshModifierEntityComponent = new AutoRuntimeClass< NavMeshModifierEntityComponent >();
	classNavMeshModifierEntityComponent->addProperty("enable", &NavMeshModifierEntityComponent::setEnable, &NavMeshModifierEntityComponent::isEnable);
	registrar->registerClass(classNavMeshModifierEntityComponent);
}

}
