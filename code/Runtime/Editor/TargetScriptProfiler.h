/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <list>
#include "Core/Ref.h"
#include "Script/IScriptProfiler.h"

namespace traktor::net
{

class BidirectionalObjectTransport;

}

namespace traktor::runtime
{

/*!
 * \ingroup Runtime
 */
class TargetScriptProfiler : public script::IScriptProfiler
{
	T_RTTI_CLASS;

public:
	explicit TargetScriptProfiler(net::BidirectionalObjectTransport* transport);

	virtual void addListener(IListener* listener) override final;

	virtual void removeListener(IListener* listener) override final;

	void notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;
};

}
