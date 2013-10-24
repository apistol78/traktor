#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessDefine.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessStep.h"

namespace traktor
{
	namespace world
	{
		namespace
		{

render::handle_t s_handleOutput;
render::handle_t s_handleInputColor;
render::handle_t s_handleInputDepth;
render::handle_t s_handleInputShadowMask;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcess", PostProcess, Object)

PostProcess::PostProcess()
:	m_requireHighRange(false)
{
	s_handleOutput = render::getParameterHandle(L"Output");
	s_handleInputColor = render::getParameterHandle(L"InputColor");
	s_handleInputDepth = render::getParameterHandle(L"InputDepth");
	s_handleInputShadowMask = render::getParameterHandle(L"InputShadowMask");
}

bool PostProcess::create(
	const PostProcessSettings* settings,
	resource::IResourceManager* resourceManager,
	render::IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height
)
{
	m_screenRenderer = new render::ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	const RefArray< PostProcessDefine >& definitions = settings->getDefinitions();
	for (RefArray< PostProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (!(*i)->define(this, renderSystem, width, height))
		{
			log::error << L"Unable to create post processing definition " << uint32_t(std::distance(definitions.begin(), i)) << Endl;
			return false;
		}
	}

	const RefArray< PostProcessStep >& steps = settings->getSteps();
	for (RefArray< PostProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		Ref< PostProcessStep::Instance > instance = (*i)->create(resourceManager, renderSystem, width, height);
		if (instance)
			m_instances.push_back(instance);
		else
		{
			log::error << L"Unable to create post processing step " << uint32_t(std::distance(steps.begin(), i)) << Endl;
			return false;
		}
	}

	m_requireHighRange = settings->requireHighRange();
	return true;
}

void PostProcess::destroy()
{
	for (RefArray< PostProcessStep::Instance >::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->destroy();

	m_instances.resize(0);

	for (SmallMap< render::handle_t, Target >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		if (
			i->second.target &&
			i->first != s_handleOutput &&
			i->first != s_handleInputColor &&
			i->first != s_handleInputDepth &&
			i->first != s_handleInputShadowMask
		)
			i->second.target->destroy();
	}
	m_targets.clear();

	if (m_screenRenderer)
	{
		m_screenRenderer->destroy();
		m_screenRenderer = 0;
	}
}

bool PostProcess::render(
	render::IRenderView* renderView,
	render::RenderTargetSet* colorBuffer,
	render::RenderTargetSet* depthBuffer,
	render::RenderTargetSet* shadowMask,
	const PostProcessStep::Instance::RenderParams& params
)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_targets[s_handleInputColor].target = colorBuffer;
	m_targets[s_handleInputDepth].target = depthBuffer;
	m_targets[s_handleInputShadowMask].target = shadowMask;
	m_currentTarget = 0;

	T_RENDER_PUSH_MARKER(renderView, "PostProcess");

	// Check if any target need to be cleared before post processing.
	for (SmallMap< render::handle_t, Target >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		Target& target = i->second;
		if (target.shouldClear)
		{
			if (renderView->begin(target.target, 0))
			{
				Color4f c(target.clearColor);
				renderView->clear(render::CfColor, &c, 0.0f, 0);
				renderView->end();
				target.shouldClear = false;
			}
		}
	}

	// Execute each post processing step in sequence.
	for (RefArray< PostProcessStep::Instance >::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		(*i)->render(
			this,
			renderView,
			m_screenRenderer,
			params
		);
	}

	T_RENDER_POP_MARKER(renderView);

	T_ASSERT_M(m_currentTarget == 0, L"Invalid post-process steps");
	return true;
}

void PostProcess::defineTarget(render::handle_t id, render::RenderTargetSet* target, const Color4f& clearColor)
{
	T_ASSERT_M(id != s_handleInputColor, L"Cannot define source color buffer");
	T_ASSERT_M(id != s_handleInputDepth, L"Cannot define source depth buffer");
	T_ASSERT_M(id != s_handleInputShadowMask, L"Cannot define source shadow mask");

	Target& t = m_targets[id];
	t.target = target;
	t.shouldClear = true;

	clearColor.storeUnaligned(t.clearColor);
}

void PostProcess::setTarget(render::IRenderView* renderView, render::handle_t id)
{
	T_ASSERT_M(id != s_handleInputColor, L"Cannot bind source color buffer as output");
	T_ASSERT_M(id != s_handleInputDepth, L"Cannot bind source depth buffer as output");
	T_ASSERT_M(id != s_handleInputShadowMask, L"Cannot bind source shadow mask as output");

	if (m_currentTarget)
		renderView->end();

	if (id != s_handleOutput)
	{
		Target& t = m_targets[id];

		m_currentTarget = t.target;
		T_ASSERT (m_currentTarget);

		renderView->begin(m_currentTarget, 0);
	}
	else
		m_currentTarget = 0;
}

render::RenderTargetSet* PostProcess::getTarget(render::handle_t id)
{
	return m_targets[id].target;
}

void PostProcess::getTargets(RefArray< render::RenderTargetSet >& outTargets) const
{
	for (SmallMap< render::handle_t, Target >::const_iterator i = m_targets.begin(); i != m_targets.end(); ++i)
		outTargets.push_back(i->second.target);
}

void PostProcess::swapTargets(render::handle_t id0, render::handle_t id1)
{
	std::swap(m_targets[id0], m_targets[id1]);
}

void PostProcess::setParameter(render::handle_t handle, bool value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_booleanParameters[handle] = value;
}

void PostProcess::setParameter(render::handle_t handle, float value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_scalarParameters[handle] = value;
}

void PostProcess::prepareShader(render::Shader* shader) const
{
	for (SmallMap< render::handle_t, bool >::const_iterator i = m_booleanParameters.begin(); i != m_booleanParameters.end(); ++i)
		shader->setCombination(i->first, i->second);
	for (SmallMap< render::handle_t, float >::const_iterator i = m_scalarParameters.begin(); i != m_scalarParameters.end(); ++i)
		shader->setFloatParameter(i->first, i->second);
}

bool PostProcess::requireHighRange() const
{
	return m_requireHighRange;
}

	}
}
