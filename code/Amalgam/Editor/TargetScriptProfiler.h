#ifndef traktor_amalgam_TargetScriptProfiler_H
#define traktor_amalgam_TargetScriptProfiler_H

#include <list>
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

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

	void notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;
};

	}
}

#endif	// traktor_amalgam_TargetScriptProfiler_H
