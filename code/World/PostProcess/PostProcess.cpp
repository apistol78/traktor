#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "World/PostProcess/PostProcess.h"
#include "World/PostProcess/PostProcessDefine.h"
#include "World/PostProcess/PostProcessSettings.h"
#include "World/PostProcess/PostProcessStep.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.world.PostProcess", PostProcess, Object)

PostProcess::PostProcess()
:	m_requireHighRange(false)
{
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
		Ref< PostProcessStep::Instance > instance = (*i)->create(resourceManager, renderSystem);
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

	for (std::map< int32_t, Ref< render::RenderTargetSet > >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		if (
			i->second &&
			i->first != PdtFrame &&
			i->first != PdtSourceColor &&
			i->first != PdtSourceDepth &&
			i->first != PdtSourceShadowMask
		)
			i->second->destroy();
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

	m_targets[PdtSourceColor] = colorBuffer;
	m_targets[PdtSourceDepth] = depthBuffer;
	m_targets[PdtSourceShadowMask] = shadowMask;
	m_currentTarget = 0;

	T_RENDER_PUSH_MARKER(renderView, "PostProcess");

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

void PostProcess::setTarget(render::IRenderView* renderView, int32_t id)
{
	T_ASSERT_M(id != PdtSourceColor, L"Cannot bind source color buffer as output");
	T_ASSERT_M(id != PdtSourceDepth, L"Cannot bind source depth buffer as output");
	T_ASSERT_M(id != PdtSourceShadowMask, L"Cannot bind source shadow mask as output");

	if (m_currentTarget)
		renderView->end();

	if (id != PdtFrame)
	{
		m_currentTarget = m_targets[id];
		renderView->begin(m_currentTarget, 0);
	}
	else
		m_currentTarget = 0;
}

Ref< render::RenderTargetSet >& PostProcess::getTargetRef(int32_t id)
{
	return m_targets[id];
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
	for (std::map< render::handle_t, bool >::const_iterator i = m_booleanParameters.begin(); i != m_booleanParameters.end(); ++i)
		shader->setCombination(i->first, i->second);
	for (std::map< render::handle_t, float >::const_iterator i = m_scalarParameters.begin(); i != m_scalarParameters.end(); ++i)
		shader->setFloatParameter(i->first, i->second);
}

bool PostProcess::requireHighRange() const
{
	return m_requireHighRange;
}

	}
}
