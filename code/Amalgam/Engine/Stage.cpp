#include "Amalgam/FrameProfiler.h"
#include "Amalgam/IEnvironment.h"
#include "Amalgam/IStateManager.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Amalgam/Engine/Layer.h"
#include "Amalgam/Engine/Stage.h"
#include "Amalgam/Engine/StageLoader.h"
#include "Amalgam/Engine/StageState.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Stage", Stage, Object)

Stage::Stage(
	amalgam::IEnvironment* environment,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const resource::Proxy< render::Shader >& shaderFade,
	const std::map< std::wstring, Guid >& transitions,
	const Object* params
)
:	m_environment(environment)
,	m_scriptContext(scriptContext)
,	m_shaderFade(shaderFade)
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
	m_environment = 0;

	if (m_scriptContext)
	{
		if (m_initialized && m_scriptContext->haveFunction("finalize"))
		{
			// Call script fini.
			script::Any argv[] =
			{
				script::Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
			};
			m_scriptContext->executeMethod(this, "finalize", sizeof_array(argv), argv);
		}

		m_scriptContext->setGlobal("stage", script::Any());
		m_scriptContext->setGlobal("environment", script::Any());

		for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			if (!(*i)->getName().empty())
				m_scriptContext->setGlobal(wstombs((*i)->getName()), script::Any());
		}

		m_scriptContext->destroy();
		m_scriptContext.clear();
	}

	safeDestroy(m_screenRenderer);

	m_shaderFade.clear();
	m_layers.clear();
	m_params = 0;
	m_pendingStage = 0;
	m_transitionStage = 0;
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

Layer* Stage::findLayer(const std::wstring& name) const
{
	for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
	{
		if ((*i)->getName() == name)
			return *i;
	}
	return 0;
}

void Stage::terminate()
{
	m_running = false;
}

script::Any Stage::invokeScript(const std::string& fn, uint32_t argc, const script::Any* argv)
{
	if (validateScriptContext())
		return m_scriptContext->executeFunction(fn, argc, argv);
	else
		return script::Any();
}

Ref< Stage > Stage::loadStage(const std::wstring& name, const Object* params)
{
	std::map< std::wstring, Guid >::const_iterator i = m_transitions.find(name);
	if (i == m_transitions.end())
	{
		log::error << L"No transition \"" << name << L"\" found" << Endl;
		return 0;
	}

	Ref< StageLoader > stageLoader = StageLoader::create(m_environment, i->second, params);
	if (stageLoader->failed())
	{
		log::error << L"Stage loader failed" << Endl;
		return 0;
	}

	return stageLoader->get();
}

Ref< StageLoader > Stage::loadStageAsync(const std::wstring& name, const Object* params)
{
	std::map< std::wstring, Guid >::const_iterator i = m_transitions.find(name);
	if (i == m_transitions.end())
	{
		log::error << L"No transition \"" << name << L"\" found" << Endl;
		return 0;
	}

	return StageLoader::createAsync(m_environment, i->second, params);
}

bool Stage::gotoStage(Stage* stage)
{
	m_pendingStage = stage;
	return true;
}

bool Stage::update(amalgam::IStateManager* stateManager, amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (!m_running)
		return false;

	if (!m_pendingStage)
	{
		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			(*i)->prepare();

		if (validateScriptContext())
		{
			info.getProfiler()->beginScope(FptScript);

			script::Any argv[] =
			{
				script::Any::fromObject(&control),
				script::Any::fromObject(const_cast< amalgam::IUpdateInfo* >(&info))
			};
			m_scriptContext->executeFunction("update", sizeof_array(argv), argv);

			info.getProfiler()->endScope();
		}

		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			(*i)->update(control, info);

		m_fade = max(0.0f, m_fade - info.getSimulationDeltaTime());
	}
	else
	{
		m_fade += info.getSimulationDeltaTime() * 1.5f;
		if (m_fade > 1.0f)
		{
			stateManager->enter(new StageState(m_environment, m_pendingStage));
			m_transitionStage = m_pendingStage;
			m_pendingStage = 0;
		}
	}

	return true;
}

bool Stage::build(const amalgam::IUpdateInfo& info, uint32_t frame)
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->build(info, frame);
	return true;
}

void Stage::render(render::EyeType eye, uint32_t frame)
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->render(eye, frame);

	if (m_shaderFade && m_fade > FUZZY_EPSILON)
	{
		m_shaderFade->setFloatParameter(L"Fade", m_fade);
		m_screenRenderer->draw(
			m_environment->getRender()->getRenderView(),
			m_shaderFade
		);
	}
}

void Stage::transition()
{
	if (!m_transitionStage)
		return;

	// Transition layers; layers of same type and name has the chance
	// to transition data, such as playing music etc.
	for (RefArray< Layer >::iterator i = m_transitionStage->m_layers.begin(); i != m_transitionStage->m_layers.end(); ++i)
	{
		std::wstring layerName = (*i)->getName();
		if (layerName.empty())
			continue;

		Layer* currentLayer = findLayer(layerName);
		if (currentLayer != 0 && &type_of(currentLayer) == &type_of(*i))
			(*i)->transition(currentLayer);
	}

	m_transitionStage = 0;
}

void Stage::preReconfigured()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->preReconfigured();
}

void Stage::postReconfigured()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->postReconfigured();
}

bool Stage::validateScriptContext()
{
	if (!m_scriptContext)
		return false;

	if (!m_initialized)
	{
		// Expose commonly used globals.
		m_scriptContext->setGlobal("stage", script::Any::fromObject(this));
		m_scriptContext->setGlobal("environment", script::Any::fromObject(m_environment));

		for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			if (!(*i)->getName().empty())
				m_scriptContext->setGlobal(wstombs((*i)->getName()), script::Any::fromObject(*i));
		}

		// Call script init; do this everytime we re-validate script.
		if (m_scriptContext->haveFunction("initialize"))
		{
			script::Any argv[] =
			{
				script::Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
			};
			m_scriptContext->executeMethod(this, "initialize", sizeof_array(argv), argv);
		}
		m_initialized = true;
	}

	return true;
}

	}
}
