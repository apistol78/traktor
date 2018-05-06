/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_amalgam_TargetScriptProfiler_H
#define traktor_amalgam_TargetScriptProfiler_H

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

	virtual void addListener(IListener* listener) T_OVERRIDE T_FINAL;

	virtual void removeListener(IListener* listener) T_OVERRIDE T_FINAL;

	void notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;
};

	}
}

#endif	// traktor_amalgam_TargetScriptProfiler_H
