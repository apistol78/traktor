#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessDefine.h"
#include "World/PostProcess/PostProcessStep.h"
#include "Render/ScreenRenderer.h"
#include "Render/RenderView.h"
#include "Render/RenderTargetSet.h"
#include "Core/Log/Log.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcess", PostProcess, Object)

bool PostProcess::create(PostProcessSettings* settings, render::RenderSystem* renderSystem, uint32_t screenWidth, uint32_t screenHeight)
{
	m_settings = settings;
	
	m_screenRenderer = gc_new< render::ScreenRenderer >();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	const RefArray< PostProcessDefine >& definitions = m_settings->getDefinitions();
	for (RefArray< PostProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (!(*i)->define(this, renderSystem, screenWidth, screenHeight))
		{
			log::error << L"Unable to create post processing definition " << uint32_t(std::distance(definitions.begin(), i)) << Endl;
			return false;
		}
	}

	const RefArray< PostProcessStep >& steps = m_settings->getSteps();
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		if (!(*i)->create(this, renderSystem))
		{
			log::error << L"Unable to create post processing step " << uint32_t(std::distance(steps.begin(), i)) << Endl;
			return false;
		}
	}

	return true;
}

void PostProcess::destroy()
{
	// Destroy steps.
	if (m_settings)
	{
		const RefArray< PostProcessStep >& steps = m_settings->getSteps();
		for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
			(*i)->destroy(this);

		m_settings = 0;
	}

	// Destroy user defined targets.
	for (std::map< uint32_t, Ref< render::RenderTargetSet > >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		if (i->second && i->first != 0 && i->first != 1)
			i->second->destroy();
	}
	m_targets.clear();

	// Destroy out screen renderer.
	if (m_screenRenderer)
	{
		m_screenRenderer->destroy();
		m_screenRenderer = 0;
	}
}

bool PostProcess::render(
	const WorldRenderView& worldRenderView,
	render::RenderView* renderView,
	render::RenderTargetSet* frameBuffer,
	render::RenderTargetSet* depthBuffer,
	float deltaTime
)
{
	if (!m_settings)
		return false;

	m_targets[1] = frameBuffer;
	m_targets[2] = depthBuffer;
	m_currentTarget = 0;

	const RefArray< PostProcessStep >& steps = m_settings->getSteps();
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		(*i)->render(
			this,
			worldRenderView,
			renderView,
			m_screenRenderer,
			deltaTime
		);
	}

	T_ASSERT_M(m_currentTarget == 0, L"Invalid post-process steps");
	return true;
}

void PostProcess::setTarget(render::RenderView* renderView, uint32_t id)
{
	T_ASSERT_M(id != 1, L"Cannot bind source color buffer as output");
	T_ASSERT_M(id != 2, L"Cannot bind source depth buffer as output");

	if (m_currentTarget)
		renderView->end();

	if (id != 0)
	{
		m_currentTarget = m_targets[id];
		renderView->begin(m_currentTarget, 0, false);
	}
	else
		m_currentTarget = 0;
}

Ref< render::RenderTargetSet >& PostProcess::getTargetRef(uint32_t id)
{
	return m_targets[id];
}

const std::map< uint32_t, Ref< render::RenderTargetSet > >& PostProcess::getTargets() const
{
	return m_targets;
}

	}
}
