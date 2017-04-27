/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Mesh/MeshComponent.h"
#include "Mesh/MeshComponentRenderer.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.MeshComponentRenderer", MeshComponentRenderer, world::IEntityRenderer)

const TypeInfoSet MeshComponentRenderer::getRenderableTypes() const
{
	TypeInfoSet typeSet;
	typeSet.insert(&type_of< MeshComponent >());
	return typeSet;
}

void MeshComponentRenderer::render(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	MeshComponent* meshComponent = checked_type_cast< MeshComponent*, false >(renderable);
	meshComponent->render(worldContext, worldRenderView, worldRenderPass);
}

void MeshComponentRenderer::flush(
	world::WorldContext& worldContext,
	world::WorldRenderView& worldRenderView,
	world::IWorldRenderPass& worldRenderPass
)
{
}

	}
}
