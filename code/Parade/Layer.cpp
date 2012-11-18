#include "Amalgam/IEnvironment.h"
#include "Amalgam/IUpdateControl.h"
#include "Amalgam/IUpdateInfo.h"
#include "Parade/Layer.h"
#include "Parade/Stage.h"
#include "Script/Any.h"
#include "Script/IScriptContext.h"

namespace traktor
{
	namespace parade
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.parade.Layer", Layer, Object)

Layer::Layer(
	Stage* stage,
	const std::wstring& name,
	const resource::Proxy< script::IScriptContext >& scriptContext
)
:	m_stage(stage)
,	m_name(name)
,	m_scriptContext(scriptContext)
,	m_initialized(false)
{
}

Layer::~Layer()
{
	destroy();
}

void Layer::destroy()
{
	if (m_scriptContext)
	{
		m_scriptContext->destroy();
		m_scriptContext.clear();
	}
	m_stage = 0;
}

void Layer::flushScript()
{
	m_initialized = false;
}

bool Layer::validateScriptContext()
{
	if (!m_scriptContext)
		return false;

	if (m_scriptContext.changed())
	{
		m_initialized = false;
		m_scriptContext.consume();
	}

	if (!m_initialized)
	{
		// Expose commonly used globals.
		m_scriptContext->setGlobal(L"stage", script::Any(m_stage));
		m_scriptContext->setGlobal(L"environment", script::Any(m_stage->getEnvironment()));

		// Call script init; do this everytime we re-validate script.
		script::Any argv[] =
		{
			script::Any((Object*)m_stage->getParams())
		};
		m_scriptContext->executeMethod(this, L"layerInit", sizeof_array(argv), argv);
		m_initialized = true;
	}

	return true;
}

script::Any Layer::invokeScriptUpdate(amalgam::IUpdateControl& control, const amalgam::IUpdateInfo& info)
{
	if (validateScriptContext())
	{
		script::Any argv[] =
		{
			script::Any(&control),
			script::Any(const_cast< amalgam::IUpdateInfo* >(&info))
		};
		return m_scriptContext->executeMethod(this, L"layerUpdate", sizeof_array(argv), argv);
	}
	else
		return script::Any();
}

script::Any Layer::invokeScriptMethod(const std::wstring& method, uint32_t argc, const script::Any* argv)
{
	if (validateScriptContext())
		return m_scriptContext->executeMethod(this, method, argc, argv);
	else
		return script::Any();
}

	}
}
