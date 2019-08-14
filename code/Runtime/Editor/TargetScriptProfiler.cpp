#include "Runtime/Editor/TargetScriptProfiler.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetScriptProfiler", TargetScriptProfiler, script::IScriptProfiler)

TargetScriptProfiler::TargetScriptProfiler(net::BidirectionalObjectTransport* transport)
:	m_transport(transport)
{
}

void TargetScriptProfiler::addListener(IListener* listener)
{
	m_listeners.push_back(listener);
}

void TargetScriptProfiler::removeListener(IListener* listener)
{
	m_listeners.remove(listener);
}

void TargetScriptProfiler::notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
{
	for (auto listener : m_listeners)
		listener->callMeasured(scriptId, function, callCount, inclusiveDuration, exclusiveDuration);
}

	}
}
