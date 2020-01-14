#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Acquire.h"
#include "Render/IRenderSystem.h"
#include "Render/IRenderTargetSet.h"
#include "Render/IRenderView.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Image/ImageProcess.h"
#include "Render/Image/ImageProcessDefine.h"
#include "Render/Image/ImageProcessData.h"
#include "Render/Image/ImageProcessStep.h"
#include "Render/Image/ImageProcessTargetPool.h"

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
	const ImageProcessData* data,
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

	m_targetPool = (targetPool != nullptr) ? targetPool : new ImageProcessTargetPool(renderSystem);
	m_requireHighRange = data->requireHighRange();
	m_allTargetsPersistent = allTargetsPersistent;

	for (auto definition : data->getDefinitions())
	{
		if (!definition->define(this, resourceManager, renderSystem, width, height))
		{
			log::error << L"Unable to create post processing definition." << Endl;
			return false;
		}
	}

	for (auto step : data->getSteps())
	{
		Ref< ImageProcessStep::Instance > instance = step->create(resourceManager, renderSystem, width, height);
		if (instance)
			m_instances.push_back(instance);
		else
		{
			log::error << L"Unable to create post processing step." << Endl;
			return false;
		}
	}

	return true;
}

void ImageProcess::destroy()
{
	for (auto instance : m_instances)
		instance->destroy();

	m_instances.resize(0);

	m_targets.clear();
	m_targetPool = nullptr;

	safeDestroy(m_screenRenderer);
}

bool ImageProcess::render(
	RenderContext* renderContext,
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

	m_currentTarget = nullptr;

	// Check if any target need to be cleared before post processing.
	for (SmallMap< handle_t, Target >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	{
		Target& target = i->second;
		if (target.rts && target.shouldClear)
		{
			auto tb = renderContext->alloc< TargetBeginRenderBlock >();
			tb->renderTargetSet = target.rts;
			tb->renderTargetIndex = -1;
			tb->clear.mask = CfColor;
			tb->clear.colors[0] = Color4f::loadUnaligned(target.clearColor);
			renderContext->enqueue(tb);

			auto te = renderContext->alloc< TargetEndRenderBlock >();
			renderContext->enqueue(te);

			target.shouldClear = false;
		}
	}

	// Create shared parameters.
	auto sharedParams = renderContext->alloc< ProgramParameters >();
	sharedParams->beginParameters(renderContext);
	for (SmallMap< handle_t, float >::const_iterator i = m_scalarParameters.begin(); i != m_scalarParameters.end(); ++i)
		sharedParams->setFloatParameter(i->first, i->second);
	for (SmallMap< handle_t, Vector4 >::const_iterator i = m_vectorParameters.begin(); i != m_vectorParameters.end(); ++i)
		sharedParams->setVectorParameter(i->first, i->second);
	for (SmallMap< handle_t, resource::Proxy< ITexture > >::const_iterator i = m_textureParameters.begin(); i != m_textureParameters.end(); ++i)
		sharedParams->setTextureParameter(i->first, i->second);
	sharedParams->endParameters(renderContext);

	// Execute each post processing step in sequence.
	for (auto instance : m_instances)
	{
		instance->render(
			this,
			renderContext,
			sharedParams,
			params
		);
	}

	// // Release all non-persistent targets.
	// for (SmallMap< handle_t, Target >::iterator i = m_targets.begin(); i != m_targets.end(); ++i)
	// {
	// 	Target& target = i->second;
	// 	if (target.rts && !target.persistent)
	// 	{
	// 		m_targetPool->releaseTarget(
	// 			target.rtscd,
	// 			target.rts
	// 		);
	// 		target.rts = nullptr;
	// 	}
	// }

	T_ASSERT_M(m_currentTarget == nullptr, L"Invalid post-process steps");
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
	t.rts = nullptr;
	t.rt = nullptr;
	t.shouldClear = persistent || m_allTargetsPersistent;
	t.persistent = persistent || m_allTargetsPersistent;
	clearColor.storeUnaligned(t.clearColor);
}

void ImageProcess::setTarget(RenderContext* renderContext, handle_t id)
{
	T_ASSERT_M(id != s_handleInputColor, L"Cannot bind source color buffer as output");
	T_ASSERT_M(id != s_handleInputDepth, L"Cannot bind source depth buffer as output");
	T_ASSERT_M(id != s_handleInputNormal, L"Cannot define source normal buffer");
	T_ASSERT_M(id != s_handleInputVelocity, L"Cannot bind source velocity buffer as output");
	T_ASSERT_M(id != s_handleInputShadowMask, L"Cannot bind source shadow mask as output");

	if (m_currentTarget)
	{
		auto te = renderContext->alloc< TargetEndRenderBlock >();
		renderContext->enqueue(te);
	}

	if (id != s_handleOutput)
	{
		Target& t = m_targets[id];

		if (t.rts == nullptr)
		{
			t.rts = m_targetPool->acquireTarget(t.rtscd);
			T_ASSERT(t.rts);

			t.rt = t.rts->getColorTexture(0);
			T_ASSERT(t.rt);
		}

		m_currentTarget = t.rts;
		T_ASSERT(m_currentTarget);

		auto tb = renderContext->alloc< TargetBeginRenderBlock >();
		tb->renderTargetSet = m_currentTarget;
		tb->renderTargetIndex = -1;
		tb->clear.mask = 0;
		renderContext->enqueue(tb);
	}
	else
		m_currentTarget = nullptr;
}

ISimpleTexture* ImageProcess::getTarget(handle_t id)
{
	Target& t = m_targets[id];

	if (t.rt == nullptr)
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
			return nullptr;
		}

		T_ASSERT(t.rts == nullptr);

		t.rts = m_targetPool->acquireTarget(t.rtscd);
		T_ASSERT(t.rts);

		t.rt = t.rts->getColorTexture(0);
		T_ASSERT(t.rt);
	}

	return t.rt;
}

void ImageProcess::swapTargets(handle_t id0, handle_t id1)
{
	std::swap(m_targets[id0], m_targets[id1]);
}

void ImageProcess::discardTarget(handle_t id)
{
	// Target& target = m_targets[id];
	// if (target.rts && !target.persistent)
	// {
	// 	m_targetPool->releaseTarget(
	// 		target.rtscd,
	// 		target.rts
	// 	);
	// 	target.rts = nullptr;
	// 	target.rt = nullptr;
	// }
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
