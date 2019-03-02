#pragma once

#include <list>
#include "Core/Ref.h"
#include "Script/IScriptProfiler.h"

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace amalgam
	{

/*! \brief
 * \ingroup Amalgam
 */
class TargetScriptProfiler : public script::IScriptProfiler
{
	T_RTTI_CLASS;

public:
	TargetScriptProfiler(net::BidirectionalObjectTransport* transport);

	virtual void addListener(IListener* listener) override final;

	virtual void removeListener(IListener* listener) override final;

	void notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;
};

	}
}

