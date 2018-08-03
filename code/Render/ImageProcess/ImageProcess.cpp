/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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
handle_t s_handleInputNormal;
handle_t s_handleInputVelocity;
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
	s_handleInputNormal = getParameterHandle(L"InputNormal");
	s_handleInputVelocity = getParameterHandle(L"InputVelocity");
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
	ISimpleTexture* colorBuffer,
	ISimpleTexture* depthBuffer,
	ISimpleTexture* normalBuffer,
	ISimpleTexture* velocityBuffer,
	ISimpleTexture* shadowMask,
	const ImageProcessStep::Instance::RenderParams& params
)
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	m_targets[s_handleInputColor].rt = colorBuffer;
	m_targets[s_handleInputColor].persistent = true;
	m_targets[s_handleInputColor].implicit = true;

	m_targets[s_handleInputDepth].rt = depthBuffer;
	m_targets[s_handleInputDepth].persistent = true;
	m_targets[s_handleInputDepth].implicit = true;

	m_targets[s_handleInputNormal].rt = normalBuffer;
	m_targets[s_handleInputNormal].persistent = true;
	m_targets[s_handleInputNormal].implicit = true;

	m_targets[s_handleInputVelocity].rt = velocityBuffer;
	m_targets[s_handleInputVelocity].persistent = true;
	m_targets[s_handleInputVelocity].implicit = true;

	m_targets[s_handleInputShadowMask].rt = shadowMask;
	m_targets[s_handleInputShadowMask].persistent = true;
	m_targets[s_handleInputShadowMask].implicit = true;

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
	T_ASSERT_M(id != s_handleInputNormal, L"Cannot define source normal buffer");
	T_ASSERT_M(id != s_handleInputVelocity, L"Cannot define source velocity buffer");
	T_ASSERT_M(id != s_handleInputShadowMask, L"Cannot define source shadow mask");
	T_ASSERT_M(rtscd.count == 1, L"rtscd.count must be 1");

	Target& t = m_targets[id];
	t.name = name;
	t.rtscd = rtscd;
	t.rts = 0;
	t.rt = 0;
	t.shouldClear = persistent || m_allTargetsPersistent;
	t.persistent = persistent || m_allTargetsPersistent;
	clearColor.storeUnaligned(t.clearColor);
}

void ImageProcess::setTarget(IRenderView* renderView, handle_t id)
{
	T_ASSERT_M(id != s_handleInputColor, L"Cannot bind source color buffer as output");
	T_ASSERT_M(id != s_handleInputDepth, L"Cannot bind source depth buffer as output");
	T_ASSERT_M(id != s_handleInputNormal, L"Cannot define source normal buffer");
	T_ASSERT_M(id != s_handleInputVelocity, L"Cannot bind source velocity buffer as output");
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

			t.rt = t.rts->getColorTexture(0);
			T_ASSERT (t.rt);
		}

		m_currentTarget = t.rts;
		T_ASSERT (m_currentTarget);

		renderView->begin(m_currentTarget, 0);
	}
	else
		m_currentTarget = 0;
}

ISimpleTexture* ImageProcess::getTarget(handle_t id)
{
	Target& t = m_targets[id];

	if (t.rt == 0)
	{
		if (t.implicit)
		{
			log::error << L"Image process using unprovided implicit target ";
			if (id == s_handleOutput)
				log::error << L"\"Output\"" << Endl;
			else if(id == s_handleInputColor)
				log::error << L"\"InputColor\"" << Endl;
			else if(id == s_handleInputDepth)
				log::error << L"\"InputDepth\"" << Endl;
			else if(id == s_handleInputNormal)
				log::error << L"\"InputDepth\"" << Endl;
			else if(id == s_handleInputVelocity)
				log::error << L"\"InputVelocity\"" << Endl;
			else if(id == s_handleInputShadowMask)
				log::error << L"\"InputShadowMask\"" << Endl;
			else
				log::error << L"unknown" << Endl;
			return 0;
		}

		T_ASSERT (t.rts == 0);

		t.rts = m_targetPool->acquireTarget(t.rtscd);
		T_ASSERT (t.rts);

		t.rt = t.rts->getColorTexture(0);
		T_ASSERT (t.rt);
	}

	return t.rt;
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
		target.rt = 0;
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
		if (i->second.rt && !i->second.name.empty())
			outTargets.push_back(DebugTarget(i->second.name, DtvDefault, i->second.rt));
	}
}

	}
}
