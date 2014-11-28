#include "Amalgam/Editor/TargetScriptProfiler.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetScriptProfiler", TargetScriptProfiler, script::IScriptProfiler)

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
	for (std::list< IListener* >::const_iterator i = m_listeners.begin(); i != m_listeners.end(); ++i)
		(*i)->callMeasured(scriptId, function, callCount, inclusiveDuration, exclusiveDuration);
}

	}
}
