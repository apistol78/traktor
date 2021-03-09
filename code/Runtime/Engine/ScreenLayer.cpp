#include "Runtime/IEnvironment.h"
#include "Runtime/Engine/ScreenLayer.h"
#include "Core/Misc/SafeDestroy.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPass.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.ScreenLayer", ScreenLayer, Layer)

ScreenLayer::ScreenLayer(
	Stage* stage,
	const std::wstring& name,
	bool permitTransition,
	IEnvironment* environment,
	const resource::Proxy< render::Shader >& shader
)
:	Layer(stage, name, permitTransition)
,	m_shader(shader)
{
	m_screenRenderer = new render::ScreenRenderer();
	m_screenRenderer->create(environment->getRender()->getRenderSystem());
}

void ScreenLayer::destroy()
{
	safeDestroy(m_screenRenderer);
	Layer::destroy();
}

void ScreenLayer::transition(Layer* fromLayer)
{
}

void ScreenLayer::prepare(const UpdateInfo& info)
{
}

void ScreenLayer::update(const UpdateInfo& info)
{
}

void ScreenLayer::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
	if (!m_shader)
		return;

	Ref< render::RenderPass > rp = new render::RenderPass(L"Screen");
	rp->setOutput(0, render::TfAll, render::TfAll);
	rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
		auto programParams = renderContext->alloc< render::ProgramParameters >();
		programParams->beginParameters(renderContext);
		if (m_parameterCallback)
			m_parameterCallback(programParams);
		programParams->endParameters(renderContext);
		m_screenRenderer->draw(renderContext, m_shader, programParams);
	});
	renderGraph.addPass(rp);
}

void ScreenLayer::preReconfigured()
{
}

void ScreenLayer::postReconfigured()
{
}

void ScreenLayer::suspend()
{
}

void ScreenLayer::resume()
{
}

	}
}
