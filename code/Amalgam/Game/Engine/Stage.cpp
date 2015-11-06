#include "Amalgam/Game/FrameProfiler.h"
#include "Amalgam/Game/IEnvironment.h"
#include "Amalgam/Game/IStateManager.h"
#include "Amalgam/Game/UpdateControl.h"
#include "Amalgam/Game/UpdateInfo.h"
#include "Amalgam/Game/Engine/Layer.h"
#include "Amalgam/Game/Engine/Stage.h"
#include "Amalgam/Game/Engine/StageLoader.h"
#include "Amalgam/Game/Engine/StageState.h"
#include "Core/Class/IRuntimeClass.h"
#include "Core/Log/Log.h"
#include "Core/Math/Const.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/TString.h"
#include "Render/ScreenRenderer.h"
#include "Render/Shader.h"
#include "Resource/IResourceManager.h"
#include "Script/IScriptContext.h"

//#define T_ENABLE_MEASURE
#include "Core/Timer/Measure.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.Stage", Stage, Object)

Stage::Stage(
	const std::wstring& name,
	IEnvironment* environment,
	const resource::Proxy< IRuntimeClass >& clazz,
	const resource::Proxy< script::IScriptContext >& scriptContext,
	const resource::Proxy< render::Shader >& shaderFade,
	float fadeRate,
	const std::map< std::wstring, Guid >& transitions,
	const Object* params
)
:	m_name(name)
,	m_environment(environment)
,	m_class(clazz)
,	m_scriptContext(scriptContext)
,	m_shaderFade(shaderFade)
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
	m_environment = 0;

	if (m_object)
	{
		uint32_t methodIdFinalize = findRuntimeClassMethodId(m_class, "finalize");
		if (m_initialized && methodIdFinalize != ~0U)
		{
			Any argv[] =
			{
				Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
			};
			m_class->invoke(m_object, methodIdFinalize, sizeof_array(argv), argv);
		}
		m_object = 0;
		m_class.clear();
	}

	if (m_scriptContext)
	{
		if (m_initialized && m_scriptContext->haveFunction("finalize"))
		{
			// Call script fini.
			Any argv[] =
			{
				Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
			};
			m_scriptContext->executeFunction("finalize", sizeof_array(argv), argv);
		}

		m_scriptContext->setGlobal("stage", Any());
		m_scriptContext->setGlobal("environment", Any());

		for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		{
			if (!(*i)->getName().empty())
				m_scriptContext->setGlobal(wstombs((*i)->getName()), Any());
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

Layer* Stage::getLayer(const std::wstring& name) const
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

Any Stage::invokeScript(const std::string& fn, uint32_t argc, const Any* argv)
{
	if (!validateScriptContext())
		return Any();

	if (m_object)
	{
		uint32_t methodId = findRuntimeClassMethodId(m_class, fn);
		if (methodId != ~0U)
			return m_class->invoke(m_object, methodId, argc, argv);
	}
	
	if (m_scriptContext)
	{
		if (m_scriptContext->haveFunction(fn))
			return m_scriptContext->executeFunction(fn, argc, argv);
	}

	log::error << L"No such script function \"" << mbstows(fn) << L"\"" << Endl;
	return Any();
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

bool Stage::update(IStateManager* stateManager, const UpdateInfo& info)
{
	T_MEASURE_BEGIN()

	if (!m_running)
		return false;

	if (!m_pendingStage)
	{
		// Set ourself as a global in shared script context.
		if (m_environment->getScript())
			m_environment->getScript()->getScriptContext()->setGlobal("stage", Any::fromObject(this));

		// Prepare all layers.
		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			T_MEASURE_STATEMENT_M((*i)->prepare(), 1.0 / 60.0, type_name(*i));

		// Issue script update.
		if (validateScriptContext())
		{
			info.getProfiler()->beginScope(FptScript);

			Any argv[] =
			{
				Any::fromObject(const_cast< UpdateInfo* >(&info))
			};

			if (m_object)
			{
				uint32_t methodIdUpdate = findRuntimeClassMethodId(m_class, "update");
				if (methodIdUpdate != ~0U)
				{
					T_MEASURE_STATEMENT(m_class->invoke(m_object, methodIdUpdate, sizeof_array(argv), argv), 1.0 / 60.0);
				}
			}

			if (m_scriptContext)
			{
				T_MEASURE_STATEMENT(m_scriptContext->executeFunction("update", sizeof_array(argv), argv), 1.0 / 60.0);
			}

			info.getProfiler()->endScope();
		}

		// Update each layer.
		for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			T_MEASURE_STATEMENT_M((*i)->update(info), 1.0 / 60.0, type_name(*i));

		// Remove ourself as a global in shared script context.
		if (m_environment->getScript())
			m_environment->getScript()->getScriptContext()->setGlobal("stage", Any());

		m_fade = max(0.0f, m_fade - info.getSimulationDeltaTime() * m_fadeRate);
	}
	else
	{
		m_fade += info.getSimulationDeltaTime() * m_fadeRate;
		if (m_fade > 1.0f)
		{
			T_MEASURE_STATEMENT(stateManager->enter(new StageState(m_environment, m_pendingStage)), 1.0 / 60.0);
			m_transitionStage = m_pendingStage;
			m_pendingStage = 0;
		}
	}

	T_MEASURE_UNTIL(1.0 / 60.0);
	return true;
}

bool Stage::build(const UpdateInfo& info, uint32_t frame)
{
	T_MEASURE_BEGIN()

	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		T_MEASURE_STATEMENT((*i)->build(info, frame), 1.0 / 60.0);

	T_MEASURE_UNTIL(1.0 / 60.0);
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

void Stage::flush()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->flush();
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

		Layer* currentLayer = getLayer(layerName);
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

	if (m_object && m_initialized)
	{
		uint32_t methodIdReconfigured = findRuntimeClassMethodId(m_class, "reconfigured");
		if (methodIdReconfigured != ~0U)
			m_class->invoke(m_object, methodIdReconfigured, 0, 0);
	}

	if (m_scriptContext && m_initialized)
	{
		if (m_scriptContext->haveFunction("reconfigured"))
			m_scriptContext->executeFunction("reconfigured");
	}
}

void Stage::suspend()
{
	if (m_object && m_initialized)
	{
		uint32_t methodIdSuspend = findRuntimeClassMethodId(m_class, "suspend");
		if (methodIdSuspend != ~0U)
			m_class->invoke(m_object, methodIdSuspend, 0, 0);
	}

	if (m_scriptContext && m_initialized)
	{
		if (m_scriptContext->haveFunction("suspend"))
			m_scriptContext->executeFunction("suspend");
	}

	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->suspend();
}

void Stage::resume()
{
	for (RefArray< Layer >::iterator i = m_layers.begin(); i != m_layers.end(); ++i)
		(*i)->resume();

	if (m_object && m_initialized)
	{
		uint32_t methodIdResume = findRuntimeClassMethodId(m_class, "resume");
		if (methodIdResume != ~0U)
			m_class->invoke(m_object, methodIdResume, 0, 0);
	}

	if (m_scriptContext && m_initialized)
	{
		if (m_scriptContext->haveFunction("resume"))
			m_scriptContext->executeFunction("resume");
	}
}

bool Stage::validateScriptContext()
{
	T_MEASURE_BEGIN()

	if (!m_class && !m_scriptContext)
		return false;

	if (!m_initialized)
	{
		if (m_class)
		{
			// Define members, do this as a prototype as we possibly want to access those in the constructor.
			IRuntimeClass::prototype_t proto;
			for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			{
				if (!(*i)->getName().empty())
					proto[wstombs((*i)->getName())] = Any::fromObject(*i);
			}

			// Call script constructor.
			Any argv[] =
			{
				Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
			};
			m_object = m_class->construct(this, sizeof_array(argv), argv, proto);
		}

		if (m_scriptContext)
		{
			// Expose commonly used globals.
			m_scriptContext->setGlobal("stage", Any::fromObject(this));
			m_scriptContext->setGlobal("environment", Any::fromObject(m_environment));

			for (RefArray< Layer >::const_iterator i = m_layers.begin(); i != m_layers.end(); ++i)
			{
				if (!(*i)->getName().empty())
					m_scriptContext->setGlobal(wstombs((*i)->getName()), Any::fromObject(*i));
			}

			// Call script init; do this everytime we re-validate script.
			if (m_scriptContext->haveFunction("initialize"))
			{
				Any argv[] =
				{
					Any::fromObject(const_cast< Object* >(m_params.c_ptr()))
				};
				m_scriptContext->executeFunction("initialize", sizeof_array(argv), argv);
			}
		}

		m_initialized = true;
	}

	T_MEASURE_UNTIL(1.0 / 60.0);
	return true;
}

	}
}
