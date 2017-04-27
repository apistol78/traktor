/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ai/AiClassFactory.h"
#include "Ai/MoveQuery.h"
#include "Ai/NavMesh.h"
#include "Ai/NavMeshEntity.h"
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/Boxes.h"
#include "Core/Class/IRuntimeClassRegistrar.h"

namespace traktor
{
	namespace ai
	{
		namespace
		{

Ref< BoxedVector4 > MoveQuery_update(MoveQuery* self, const Vector4& currentPosition)
{
	Vector4 moveToPosition;
	if (self->update(currentPosition, moveToPosition))
		return new BoxedVector4(moveToPosition);
	else
		return 0;
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

const NavMesh* NavMeshEntity_get(NavMeshEntity* self)
{
	return self->get();
}

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.ai.AiClassFactory", 0, AiClassFactory, IRuntimeClassFactory)

void AiClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	Ref< AutoRuntimeClass< MoveQuery > > classMoveQuery = new AutoRuntimeClass< MoveQuery >();
	classMoveQuery->addMethod("update", &MoveQuery_update);
	registrar->registerClass(classMoveQuery);

	Ref< AutoRuntimeClass< NavMesh > > classNavMesh = new AutoRuntimeClass< NavMesh >();
	classNavMesh->addMethod("createMoveQuery", &NavMesh::createMoveQuery);
	classNavMesh->addMethod("findClosestPoint", &NavMesh_findClosestPoint);
	classNavMesh->addMethod("findRandomPoint", &NavMesh_findRandomPoint_1);
	classNavMesh->addMethod("findRandomPoint", &NavMesh_findRandomPoint_2);
	registrar->registerClass(classNavMesh);

	Ref< AutoRuntimeClass< NavMeshEntity > > classNavMeshEntity = new AutoRuntimeClass< NavMeshEntity >();
	classNavMeshEntity->addMethod("get", &NavMeshEntity_get);
	registrar->registerClass(classNavMeshEntity);
}

	}
}
