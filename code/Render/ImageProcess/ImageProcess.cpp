#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderView.h"
#include "Render/RenderTargetSet.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/ImageProcess/ImageProcess.h"
#include "Render/ImageProcess/ImageProcessDefine.h"
#include "Render/ImageProcess/ImageProcessSettings.h"
#include "Render/ImageProcess/ImageProcessStep.h"
#include "Render/ImageProcess/ImageProcessTargetPool.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

handle_t s_handleOutput;
handle_t s_handleInputColor;
handle_t s_handleInputDepth;
handle_t s_handleInputShadowMask;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.ImageProcess", ImageProcess, Object)

ImageProcess::ImageProcess()
:	m_requireHighRange(false)
,	m_allTargetsPersistent(false)
{
	s_handleOutput = getParameterHandle(L"Output");
	s_handleInputColor = getParameterHandle(L"InputColor");
	s_handleInputDepth = getParameterHandle(L"InputDepth");
	s_handleInputShadowMask = getParameterHandle(L"InputShadowMask");
}

bool ImageProcess::create(
	const ImageProcessSettings* settings,
	ImageProcessTargetPool* targetPool,
	resource::IResourceManager* resourceManager,
	IRenderSystem* renderSystem,
	uint32_t width,
	uint32_t height,
	bool allTargetsPersistent
)
{
	m_screenRenderer = new ScreenRenderer();
	if (!m_screenRenderer->create(renderSystem))
		return false;

	m_targetPool = (targetPool != 0) ? targetPool : new ImageProcessTargetPool(renderSystem);
	m_requireHighRange = settings->requireHighRange();
	m_allTargetsPersistent = allTargetsPersistent;

	const RefArray< ImageProcessDefine >& definitions = settings->getDefinitions();
	for (RefArray< ImageProcessDefine >::const_iterator i = definitions.begin(); i != definitions.end(); ++i)
	{
		if (!(*i)->define(this, resourceManager, renderSystem, width, height))
		{
			log::error << L"Unable to create post processing definition " << uint32_t(std::distance(definitions.begin(), i)) << Endl;
			return false;
		}
	}

	const RefArray< ImageProcessStep >& steps = settings->getSteps();
	for (RefArray< ImageProcessStep >::const_iterator i = steps.begin(); i != steps.end(); ++i)
	{
		Ref< ImageProcessStep::Instance > instance = (*i)->create(resourceManager, renderSystem, width, height);
		if (instance)
			m_instances.push_back(instance);
		else
		{
			log::error << L"Unable to create post processing step " << uint32_t(std::distance(steps.begin(), i)) << Endl;
			return false;
		}
	}

	return true;
}

void ImageProcess::destroy()
{
	for (RefArray< ImageProcessStep::Instance >::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
		(*i)->destroy();

	m_instances.resize(0);

	m_targets.clear();
	m_targetPool = 0;

	if (m_screenRenderer)
	{
		m_screenRenderer->destroy();
		m_screenRenderer = 0;
	}
}

bool ImageProcess::render(
	IRenderView* renderView,
	RenderTargetSet* colorBuffer,
	RenderTargetSet* depthBuffer,
	RenderTargetSet* shadowMask,
	const ImageProcessStep::Instance::RenderParams& params
)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_targets[s_handleInputColor].rts = colorBuffer;
	m_targets[s_handleInputColor].persistent = true;

	m_targets[s_handleInputDepth].rts = depthBuffer;
	m_targets[s_handleInputDepth].persistent = true;

	m_targets[s_handleInputShadowMask].rts = shadowMask;
	m_targets[s_handleInputShadowMask].persistent = true;

	m_currentTarget = 0;

	T_RENDER_PUSH_MARKER(renderView, "ImageProcess");

	// Check if any target need to be cleared before post processing.
	for (SmallMap< handle_t, Target >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		Target& target = i->second;
		if (target.rts && target.shouldClear)
		{
			if (renderView->begin(target.rts, 0))
			{
				Color4f c(target.clearColor);
				renderView->clear(CfColor, &c, 0.0f, 0);
				renderView->end();
				target.shouldClear = false;
			}
		}
	}

	// Execute each post processing step in sequence.
	for (RefArray< ImageProcessStep::Instance >::const_iterator i = m_instances.begin(); i != m_instances.end(); ++i)
	{
		(*i)->render(
			this,
			renderView,
			m_screenRenderer,
			params
		);
	}

	// Release all non-persistent targets.
	for (SmallMap< handle_t, Target >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		Target& target = i->second;
		if (target.rts && !target.persistent)
		{
			m_targetPool->releaseTarget(
				target.rtscd,
				target.rts
			);
			target.rts = 0;
		}
	}

	T_RENDER_POP_MARKER(renderView);

	T_ASSERT_M(m_currentTarget == 0, L"Invalid post-process steps");
	return true;
}

void ImageProcess::defineTarget(const std::wstring& name, handle_t id, const RenderTargetSetCreateDesc& rtscd, const Color4f& clearColor, bool persistent)
{
	T_ASSERT_M(id != s_handleInputColor, L"Cannot define source color buffer");
	T_ASSERT_M(id != s_handleInputDepth, L"Cannot define source depth buffer");
	T_ASSERT_M(id != s_handleInputShadowMask, L"Cannot define source shadow mask");

	Target& t = m_targets[id];
	t.name = name;
	t.rtscd = rtscd;
	t.rts = 0;
	t.shouldClear = persistent || m_allTargetsPersistent;
	t.persistent = persistent || m_allTargetsPersistent;
	clearColor.storeUnaligned(t.clearColor);
}

void ImageProcess::setTarget(IRenderView* renderView, handle_t id)
{
	T_ASSERT_M(id != s_handleInputColor, L"Cannot bind source color buffer as output");
	T_ASSERT_M(id != s_handleInputDepth, L"Cannot bind source depth buffer as output");
	T_ASSERT_M(id != s_handleInputShadowMask, L"Cannot bind source shadow mask as output");

	if (m_currentTarget)
		renderView->end();

	if (id != s_handleOutput)
	{
		Target& t = m_targets[id];

		if (t.rts == 0)
		{
			t.rts = m_targetPool->acquireTarget(t.rtscd);
			T_ASSERT (t.rts);
		}

		m_currentTarget = t.rts;
		T_ASSERT (m_currentTarget);

		renderView->begin(m_currentTarget, 0);
	}
	else
		m_currentTarget = 0;
}

RenderTargetSet* ImageProcess::getTarget(handle_t id)
{
	Target& t = m_targets[id];

	if (t.rts == 0)
	{
		t.rts = m_targetPool->acquireTarget(t.rtscd);
		T_ASSERT (t.rts);
	}

	return t.rts;
}

void ImageProcess::swapTargets(handle_t id0, handle_t id1)
{
	std::swap(m_targets[id0], m_targets[id1]);
}

void ImageProcess::discardTarget(handle_t id)
{
	Target& target = m_targets[id];
	if (target.rts && !target.persistent)
	{
		m_targetPool->releaseTarget(
			target.rtscd,
			target.rts
		);
		target.rts = 0;
	}
}

void ImageProcess::setCombination(handle_t handle, bool value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_booleanParameters[handle] = value;
}

void ImageProcess::setFloatParameter(handle_t handle, float value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_scalarParameters[handle] = value;
}

void ImageProcess::setVectorParameter(handle_t handle, const Vector4& value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_vectorParameters[handle] = value;
}

void ImageProcess::setTextureParameter(handle_t handle, const resource::Proxy< ITexture >& value)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	m_textureParameters[handle] = value;
}

void ImageProcess::prepareShader(Shader* shader) const
{
	for (SmallMap< handle_t, bool >::const_iterator i = m_booleanParameters.begin(); i != m_booleanParameters.end(); ++i)
		shader->setCombination(i->first, i->second);
	for (SmallMap< handle_t, float >::const_iterator i = m_scalarParameters.begin(); i != m_scalarParameters.end(); ++i)
		shader->setFloatParameter(i->first, i->second);
	for (SmallMap< handle_t, Vector4 >::const_iterator i = m_vectorParameters.begin(); i != m_vectorParameters.end(); ++i)
		shader->setVectorParameter(i->first, i->second);
	for (SmallMap< handle_t, resource::Proxy< ITexture > >::const_iterator i = m_textureParameters.begin(); i != m_textureParameters.end(); ++i)
		shader->setTextureParameter(i->first, i->second);
}

bool ImageProcess::requireHighRange() const
{
	return m_requireHighRange;
}

void ImageProcess::getDebugTargets(std::vector< DebugTarget >& outTargets) const
{
	for (SmallMap< handle_t, Target >::const_iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		if (i->second.rts && !i->second.name.empty())
			outTargets.push_back(DebugTarget(i->second.name, DtvDefault, i->second.rts->getColorTexture(0)));
	}
}

	}
}
