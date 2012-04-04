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
	const std::wstring& name,
	const resource::Proxy< script::IScriptContext >& scriptContext
)
:	m_name(name)
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
	if (m_scriptContext.valid())
	{
		m_scriptContext->destroy();
		m_scriptContext.clear();
	}
}

void Layer::flushScript()
{
	m_initialized = false;
}

bool Layer::validateScriptContext(Stage* stage)
{
	if (!m_scriptContext.valid())
	{
		if (!m_scriptContext.validate())
			return false;

		m_initialized = false;
	}

	if (!m_initialized)
	{
		// Call script init; do this everytime we re-validate script.
		script::Any argv[] =
		{
			script::Any(stage),
			script::Any((Object*)stage->getParams())
		};
		m_scriptContext->executeMethod(this, L"layerInit", sizeof_array(argv), argv);
		m_initialized = true;
	}

	return true;
}

void Layer::invokeScriptUpdate(Stage* stage, const amalgam::IUpdateInfo& info)
{
	if (!validateScriptContext(stage))
		return;

	script::Any argv[] =
	{
		script::Any(stage),
		script::Any(const_cast< amalgam::IUpdateInfo* >(&info))
	};
	m_scriptContext->executeMethod(this, L"layerUpdate", sizeof_array(argv), argv);
}

void Layer::invokeScriptMethod(Stage* stage, const std::wstring& method, uint32_t argc, const script::Any* argv)
{
	if (!validateScriptContext(stage))
		return;

	m_scriptContext->executeMethod(this, method, argc, argv);
}

	}
}
