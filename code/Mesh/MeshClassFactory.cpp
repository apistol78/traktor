/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/AutoRuntimeClass.h"
#include "Core/Class/IRuntimeClassRegistrar.h"
#include "Core/Class/IRuntimeDelegate.h"
#include "Core/Class/Boxes/BoxedAlignedVector.h"
#include "Core/Class/Boxes/BoxedIntervalTransform.h"
#include "Mesh/MeshClassFactory.h"
#include "Mesh/MeshComponent.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshComponent.h"
#include "Mesh/Skinned/SkinnedMesh.h"
#include "Mesh/Skinned/SkinnedMeshComponent.h"
#include "Mesh/Static/StaticMesh.h"
#include "Mesh/Static/StaticMeshComponent.h"
#include "Render/Buffer.h"
#include "Render/ITexture.h"
#include "Render/RenderClassFactory.h"
#include "Render/Context/ProgramParameters.h"
#include "Resource/ResourceCast.h"

namespace traktor::mesh
{
	namespace
	{

const IntervalTransform& MeshComponent_getTransform(MeshComponent* self)
{
	return self->getTransform();
}

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.MeshClassFactory", 0, MeshClassFactory, IRuntimeClassFactory)

void MeshClassFactory::createClasses(IRuntimeClassRegistrar* registrar) const
{
	auto classIMesh = new AutoRuntimeClass< IMesh >();
	registrar->registerClass(classIMesh);

	auto classMeshComponent = new AutoRuntimeClass< MeshComponent >();
	classMeshComponent->addProperty("transform", &MeshComponent_getTransform);
	registrar->registerClass(classMeshComponent);

	auto classInstanceMesh = new AutoRuntimeClass< InstanceMesh >();
	registrar->registerClass(classInstanceMesh);

	auto classInstanceMeshComponent = new AutoRuntimeClass< InstanceMeshComponent >();
	registrar->registerClass(classInstanceMeshComponent);

	auto classSkinnedMesh = new AutoRuntimeClass< SkinnedMesh >();
	registrar->registerClass(classSkinnedMesh);

	auto classSkinnedMeshComponent = new AutoRuntimeClass< SkinnedMeshComponent >();
	registrar->registerClass(classSkinnedMeshComponent);

	auto classStaticMesh = new AutoRuntimeClass< StaticMesh >();
	registrar->registerClass(classStaticMesh);

	auto classStaticMeshComponent = new AutoRuntimeClass< StaticMeshComponent >();
	registrar->registerClass(classStaticMeshComponent);
}

}
