/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Spray/Effect.h"
#include "Spray/EffectComponent.h"
#include "Spray/EffectRenderer.h"
#include "Spray/MeshRenderer.h"
#include "Spray/PointRenderer.h"
#include "Spray/TrailRenderer.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldBuildContext.h"
#include "World/WorldRenderView.h"

namespace traktor::spray
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.EffectRenderer", EffectRenderer, world::IEntityRenderer)

EffectRenderer::EffectRenderer(render::IRenderSystem* renderSystem, float lod1Distance, float lod2Distance)
:	m_pointRenderer(new PointRenderer(renderSystem, lod1Distance, lod2Distance))
,	m_meshRenderer(new MeshRenderer())
,	m_trailRenderer(new TrailRenderer(renderSystem))
{
}

void EffectRenderer::setLodDistances(float lod1Distance, float lod2Distance)
{
	m_pointRenderer->setLodDistances(lod1Distance, lod2Distance);
}

bool EffectRenderer::initialize(const ObjectStore& objectStore)
{
	return true;
}

const TypeInfoSet EffectRenderer::getRenderableTypes() const
{
	return makeTypeInfoSet< EffectComponent >();
}

void EffectRenderer::setup(
	const world::WorldSetupContext& context,
	const world::WorldRenderView& worldRenderView,
	Object* renderable
)
{
}

void EffectRenderer::setup(
	const world::WorldSetupContext& context
)
{
}

void EffectRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass,
	Object* renderable
)
{
	auto effectComponent = static_cast< const EffectComponent* >(renderable);

	// Do we need to render anything with this technique?
	if (!effectComponent->haveTechnique(worldRenderPass.getTechnique()))
		return;

	const Aabb3 boundingBox = effectComponent->getWorldBoundingBox();
	if (boundingBox.empty())
		return;

	// Early out of bounding sphere is outside of frustum.
	const Vector4 center = worldRenderView.getView() * boundingBox.getCenter().xyz1();
	const Scalar radius = boundingBox.getExtent().length();
	if (worldRenderView.getCullFrustum().inside(center, radius) == Frustum::Result::Outside)
		return;

	const Vector4 cameraPosition = worldRenderView.getEyePosition();
	const Plane cameraPlane(worldRenderView.getEyeDirection(), cameraPosition);

	effectComponent->render(
		worldRenderPass.getTechnique(),
		cameraPosition,
		cameraPlane,
		m_pointRenderer,
		m_meshRenderer,
		m_trailRenderer
	);
}

void EffectRenderer::build(
	const world::WorldBuildContext& context,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	m_pointRenderer->flush(context.getRenderContext(), worldRenderPass);
	m_meshRenderer->flush(context.getRenderContext(), worldRenderPass);
	m_trailRenderer->flush(context.getRenderContext(), worldRenderPass);
}

}
