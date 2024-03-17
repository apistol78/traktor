/*
 * TRAKTOR
 * Copyright (c) 2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Misc/SafeDestroy.h"

#include "Render/PrimitiveRenderer.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"

#include "Runtime/IEnvironment.h"
#include "Runtime/IRenderServer.h"
#include "Runtime/IResourceServer.h"

#include "Runtime/Engine/DebugLayer.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/WorldLayer.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.DebugLayer", DebugLayer, Layer)

DebugLayer::DebugLayer(
	Stage* stage,
	IEnvironment* environment
)
:	Layer(stage, L"debug", false)
{
	m_primitiveRenderer = new render::PrimitiveRenderer();
	m_primitiveRenderer->create(
		environment->getResource()->getResourceManager(),
		environment->getRender()->getRenderSystem(),
		16
	);
}

void DebugLayer::destroy()
{
	Layer::destroy();
	safeDestroy(m_primitiveRenderer);
}

void DebugLayer::transition(Layer* fromLayer)
{
}

void DebugLayer::preUpdate(const UpdateInfo& info)
{
	m_points.resize(0);
	m_lines.resize(0);
}

void DebugLayer::update(const UpdateInfo& info)
{
}

void DebugLayer::postUpdate(const UpdateInfo& info)
{
}

void DebugLayer::preSetup(const UpdateInfo& info)
{
}

void DebugLayer::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
	WorldLayer* world = dynamic_type_cast< WorldLayer* >(getStage()->getLayer(L"world"));
	if (!world)
		return;

	const Matrix44 projection = world->getWorldRenderView().getProjection();
	const Matrix44 view = world->getWorldRenderView().getView();

	m_primitiveRenderer->begin(m_count, projection);
	m_primitiveRenderer->setClipDistance(world->getWorldRenderView().getViewFrustum().getNearZ());
	m_primitiveRenderer->pushView(view);
	m_primitiveRenderer->pushDepthState(false, false, false);

	for (const auto& point : m_points)
	{
		m_primitiveRenderer->drawSolidPoint(
			point.position.xyz1(), 
			point.size,
			point.color.toColor4ub()
		);
	}

	for (const auto& line : m_lines)
	{
		m_primitiveRenderer->drawLine(
			line.from.xyz1(), 
			line.to.xyz1(), 
			line.width,
			line.color.toColor4ub()
		);
	}

	m_primitiveRenderer->popDepthState();
	m_primitiveRenderer->popView();
	m_primitiveRenderer->end(m_count);

	Ref< render::RenderPass > rp = new render::RenderPass(L"Debug");
	rp->setOutput(0, render::TfAll, render::TfAll);
	rp->addBuild([=, count = m_count, points = m_points](const render::RenderGraph&, render::RenderContext* renderContext) {
		auto rb = renderContext->allocNamed< render::LambdaRenderBlock >(L"Debug wire");
		rb->lambda = [=](render::IRenderView* renderView) {
			m_primitiveRenderer->render(renderView, count);
		};
		renderContext->draw(rb);
	});
	renderGraph.addPass(rp);

	m_count = (m_count + 1) % 16;
}

void DebugLayer::preReconfigured()
{
}

void DebugLayer::postReconfigured()
{
}

void DebugLayer::suspend()
{
}

void DebugLayer::resume()
{
}

void DebugLayer::hotReload()
{
}

void DebugLayer::point(const Vector4& position, float size, const Color4f& color)
{
	m_points.push_back({ position, color, size });
}

void DebugLayer::line(const Vector4& from, const Vector4& to, float width, const Color4f& color)
{
	m_lines.push_back({ from, to, color, width });
}

}
