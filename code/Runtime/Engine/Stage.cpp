/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Class/IRuntimeClass.h"
#include "Core/Class/IRuntimeDispatch.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Core/Timer/Profiler.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Render/Context/RenderContext.h"
#include "Render/Frame/RenderGraph.h"
#include "Render/Frame/RenderPass.h"
#include "Resource/IResourceManager.h"
#include "Runtime/IEnvironment.h"
#include "Runtime/IStateManager.h"
#include "Runtime/UpdateControl.h"
#include "Runtime/UpdateInfo.h"
#include "Runtime/Engine/Layer.h"
#include "Runtime/Engine/Stage.h"
#include "Runtime/Engine/StageLoader.h"
#include "Runtime/Engine/StageState.h"

namespace traktor::runtime
{
	namespace
	{

const render::Handle c_handleFade(L"Fade");

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.Stage", Stage, Object)

Stage::Stage(
	const std::wstring& name,
	IEnvironment* environment,
	const resource::Proxy< IRuntimeClass >& clazz,
	const resource::Proxy< render::Shader >& shaderFade,
	bool fadeOutUpdate,
	float fadeRate,
	const SmallMap< std::wstring, Guid >& transitions,
	const Object* params
)
:	m_name(name)
,	m_environment(environment)
,	m_class(clazz)
,	m_shaderFade(shaderFade)
,	m_fadeOutUpdate(fadeOutUpdate)
,	m_fadeRate(fadeRate)
,	m_transitions(transitions)
,	m_params(params)
,	m_initialized(false)
,	m_running(true)
,	m_fade(1.0f)
{
	m_screenRenderer = new render::ScreenRenderer();
	m_screenRenderer->create(m_environment->getRender()->getRenderSystem());
}

Stage::~Stage()
{
	destroy();
}

void Stage::destroy()
{
	m_environment = nullptr;

	if (m_object)
	{
		const IRuntimeDispatch* methodFinalize = findRuntimeClassMethod(m_class, "finalize");
		if (m_initialized && methodFinalize != nullptr)
		{
			const Any argv[] =
			{
				Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
			};
			methodFinalize->invoke(m_object, sizeof_array(argv), argv);
		}
		m_object = nullptr;
		m_class.clear();
	}

	safeDestroy(m_screenRenderer);

	m_shaderFade.clear();

	for (auto layer : m_layers)
		layer->destroy();

	m_layers.clear();

	m_params = nullptr;
	m_pendingStage = nullptr;
	m_transitionStage = nullptr;
}

void Stage::addLayer(Layer* layer)
{
	m_layers.push_back(layer);
}

void Stage::removeLayer(Layer* layer)
{
	m_layers.remove(layer);
}

void Stage::removeAllLayers()
{
	m_layers.resize(0);
}

Layer* Stage::getLayer(const std::wstring& name) const
{
	for (auto layer : m_layers)
	{
		if (layer->getName() == name)
			return layer;
	}
	return nullptr;
}

void Stage::terminate()
{
	m_running = false;
}

Any Stage::invokeScript(const std::string& fn, uint32_t argc, const Any* argv)
{
	T_PROFILER_SCOPE(L"Script invoke");

	if (!validateScriptContext())
		return Any();

	if (m_object)
	{
		const IRuntimeDispatch* method = findRuntimeClassMethod(m_class, fn);
		if (method != nullptr)
			return method->invoke(m_object, argc, argv);
	}

	log::error << L"No such script function \"" << mbstows(fn) << L"\"." << Endl;
	return Any();
}

bool Stage::haveTransition(const std::wstring& name) const
{
	return m_transitions.find(name) != m_transitions.end();
}

Ref< Stage > Stage::loadStage(const std::wstring& name, const Object* params)
{
	auto it = m_transitions.find(name);
	if (it == m_transitions.end())
	{
		log::error << L"No transition \"" << name << L"\" found." << Endl;
		return nullptr;
	}

	Ref< StageLoader > stageLoader = StageLoader::create(m_environment, it->second, params);
	if (stageLoader->failed())
	{
		log::error << L"Stage loader failed." << Endl;
		return nullptr;
	}

	return stageLoader->get();
}

Ref< StageLoader > Stage::loadStageAsync(const std::wstring& name, const Object* params)
{
	auto it = m_transitions.find(name);
	if (it == m_transitions.end())
	{
		log::error << L"No transition \"" << name << L"\" found." << Endl;
		return nullptr;
	}

	return StageLoader::createAsync(m_environment, it->second, params);
}

bool Stage::gotoStage(Stage* stage)
{
	m_pendingStage = stage;
	return true;
}

bool Stage::update(IStateManager* stateManager, const UpdateInfo& info)
{
	if (!m_running)
		return false;

	bool updateValid = false;

	if (!m_pendingStage)
	{
		m_fade = max(0.0f, m_fade - info.getSimulationDeltaTime() * m_fadeRate);
		updateValid = true;
	}
	else
	{
		// Next stage ready, fade out.
		m_fade += info.getSimulationDeltaTime() * m_fadeRate;
		if (m_fade > 1.0f)
		{
			// Fade out finished; enter next stage.
			stateManager->enter(new StageState(m_environment, m_pendingStage));
			m_transitionStage = m_pendingStage;
			m_pendingStage = nullptr;
			updateValid = false;
		}
		else
		{
			// Fade out in progress; update valid is configurable.
			updateValid = m_fadeOutUpdate;
		}
	}

	if (updateValid)
	{
		// Prepare all layers.
		for (auto layer : m_layers)
			layer->preUpdate(info);

		// Issue script update.
		if (validateScriptContext())
		{
			T_PROFILER_SCOPE(L"Script update");
			if (m_object)
			{
				const Any argv[] =
				{
					Any::fromObject(const_cast< UpdateInfo* >(&info))
				};

				const IRuntimeDispatch* methodUpdate = findRuntimeClassMethod(m_class, "update");
				if (methodUpdate != nullptr)
					methodUpdate->invoke(m_object, sizeof_array(argv), argv);
			}
		}

		// Update each layer.
		for (auto layer : m_layers)
			layer->update(info);

		// Issue script post update.
		if (validateScriptContext())
		{
			T_PROFILER_SCOPE(L"Script post update");
			if (m_object)
			{
				const Any argv[] =
				{
					Any::fromObject(const_cast< UpdateInfo* >(&info))
				};

				const IRuntimeDispatch* methodPostUpdate = findRuntimeClassMethod(m_class, "postUpdate");
				if (methodPostUpdate != nullptr)
					methodPostUpdate->invoke(m_object, sizeof_array(argv), argv);
			}
		}
	}

	return true;
}

bool Stage::setup(const UpdateInfo& info, render::RenderGraph& renderGraph)
{
	// Prepare all layers.
	for (auto layer : m_layers)
		layer->preSetup(info);

	// Issue script post update.
	if (validateScriptContext())
	{
		T_PROFILER_SCOPE(L"Script setup");
		if (m_object)
		{
			const Any argv[] =
			{
				Any::fromObject(const_cast< UpdateInfo* >(&info))
			};

			const IRuntimeDispatch* methodSetup = findRuntimeClassMethod(m_class, "setup");
			if (methodSetup != nullptr)
				methodSetup->invoke(m_object, sizeof_array(argv), argv);
		}
	}

	for (auto layer : m_layers)
		layer->setup(info, renderGraph);

	 if (m_shaderFade && m_fade > FUZZY_EPSILON)
	 {
		Ref< render::RenderPass > rp = new render::RenderPass(L"Fade");
		rp->setOutput(0, render::TfAll, render::TfAll);
		rp->addBuild([&](const render::RenderGraph&, render::RenderContext* renderContext) {
			auto programParams = renderContext->alloc< render::ProgramParameters >();
			programParams->beginParameters(renderContext);
			programParams->setFloatParameter(c_handleFade, m_fade);
			programParams->endParameters(renderContext);
			m_screenRenderer->draw(renderContext, m_shaderFade, programParams);
		});
		renderGraph.addPass(rp);
	 }

	return true;
}

void Stage::transition()
{
	if (!m_transitionStage)
		return;

	// Transition layers; layers of same type and name has the chance
	// to transition data, such as playing music etc.
	for (auto layer : m_transitionStage->m_layers)
	{
		const std::wstring layerName = layer->getName();
		if (layerName.empty())
			continue;

		Layer* currentLayer = getLayer(layerName);
		if (currentLayer != nullptr && currentLayer != layer && &type_of(currentLayer) == &type_of(layer))
			layer->transition(currentLayer);
	}

	m_transitionStage = nullptr;
}

void Stage::preReconfigured()
{
	for (auto layer : m_layers)
		layer->preReconfigured();
}

void Stage::postReconfigured()
{
	for (auto layer : m_layers)
		layer->postReconfigured();

	if (m_object && m_initialized)
	{
		const IRuntimeDispatch* methodReconfigured = findRuntimeClassMethod(m_class, "reconfigured");
		if (methodReconfigured != nullptr)
			methodReconfigured->invoke(m_object, 0, nullptr);
	}
}

void Stage::suspend()
{
	if (m_object && m_initialized)
	{
		const IRuntimeDispatch* methodSuspend = findRuntimeClassMethod(m_class, "suspend");
		if (methodSuspend != nullptr)
			methodSuspend->invoke(m_object, 0, nullptr);
	}

	for (auto layer : m_layers)
		layer->suspend();
}

void Stage::resume()
{
	for (auto layer : m_layers)
		layer->resume();

	if (m_object && m_initialized)
	{
		const IRuntimeDispatch* methodResume = findRuntimeClassMethod(m_class, "resume");
		if (methodResume != nullptr)
			methodResume->invoke(m_object, 0, nullptr);
	}
}

bool Stage::validateScriptContext()
{
	if (!m_class)
		return false;

	if (!m_initialized)
	{
		T_PROFILER_SCOPE(L"Script validate");
		if (m_class)
		{
			// Call script constructor.
			const Any argv[] =
			{
				Any::fromObject(const_cast< Object* >(m_params.c_ptr())),
				Any::fromObject(m_environment)
			};
			m_object = createRuntimeClassInstance(m_class, this, sizeof_array(argv), argv);
		}
		m_initialized = true;
	}

	return true;
}

}
