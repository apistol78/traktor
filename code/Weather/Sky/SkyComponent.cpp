/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <limits>
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/Buffer.h"
#include "Render/ITexture.h"
#include "Render/Context/RenderContext.h"
#include "Weather/Sky/SkyComponent.h"
#include "World/IWorldRenderPass.h"
#include "World/WorldRenderView.h"

namespace traktor::weather
{
	namespace
	{

const render::Handle s_handleWeather_SkyRadius(L"Weather_SkyRadius");
const render::Handle s_handleWeather_SkyRotation(L"Weather_SkyRotation");
const render::Handle s_handleWeather_SkyTexture(L"Weather_SkyTexture");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.weather.SkyComponent", SkyComponent, world::IEntityComponent)

SkyComponent::SkyComponent(
	const render::IVertexLayout* vertexLayout,
	render::Buffer* vertexBuffer,
	render::Buffer* indexBuffer,
	const render::Primitives& primitives,
	const resource::Proxy< render::Shader >& shader,
	const resource::Proxy< render::ITexture >& texture
)
:	m_vertexLayout(vertexLayout)
,	m_vertexBuffer(vertexBuffer)
,	m_indexBuffer(indexBuffer)
,	m_primitives(primitives)
,	m_shader(shader)
,	m_texture(texture)
,	m_transform(Transform::identity())
{
}

SkyComponent::~SkyComponent()
{
	destroy();
}

void SkyComponent::destroy()
{
	safeDestroy(m_indexBuffer);
	safeDestroy(m_vertexBuffer);
}

void SkyComponent::setOwner(world::Entity* owner)
{
}

void SkyComponent::setTransform(const Transform& transform)
{
	m_transform = transform;
}

Aabb3 SkyComponent::getBoundingBox() const
{
	const float c_radius = 1e4f;
	return Aabb3(Vector4(-c_radius, -c_radius, -c_radius, 1.0f), Vector4(c_radius, c_radius, c_radius, 1.0f));
}

void SkyComponent::update(const world::UpdateParams& update)
{
}

void SkyComponent::build(
	render::RenderContext* renderContext,
	const world::WorldRenderView& worldRenderView,
	const world::IWorldRenderPass& worldRenderPass
)
{
	auto sp = worldRenderPass.getProgram(m_shader);
	if (!sp)
		return;

	const float rotation = m_transform.rotation().toEulerAngles().x();

	auto renderBlock = renderContext->alloc< render::SimpleRenderBlock >(L"Sky");

	// Render sky after all opaques but before of all alpha blended.
	renderBlock->distance = std::numeric_limits< float >::max();
	renderBlock->program = sp.program;
	renderBlock->programParams = renderContext->alloc< render::ProgramParameters >();
	renderBlock->indexBuffer = m_indexBuffer->getBufferView();
	renderBlock->indexType = render::IndexType::UInt16;
	renderBlock->vertexBuffer = m_vertexBuffer->getBufferView();
	renderBlock->vertexLayout = m_vertexLayout;
	renderBlock->primitives = m_primitives;

	renderBlock->programParams->beginParameters(renderContext);

	worldRenderPass.setProgramParameters(renderBlock->programParams);

	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyRadius, worldRenderView.getViewFrustum().getFarZ() - 10.0f);
	renderBlock->programParams->setFloatParameter(s_handleWeather_SkyRotation, rotation);
	renderBlock->programParams->setTextureParameter(s_handleWeather_SkyTexture, m_texture);

	renderBlock->programParams->endParameters(renderContext);

	renderContext->draw(sp.priority, renderBlock);
}

}
