/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Ref.h"
#include "Core/Containers/AlignedVector.h"
#include "Script/IScriptProfiler.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SCRIPT_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::net
{

class BidirectionalObjectTransport;

}

namespace traktor::script
{

/*!
 * \ingroup Script
 */
class T_DLLCLASS RemoteScriptProfiler : public script::IScriptProfiler
{
	T_RTTI_CLASS;

public:
	explicit RemoteScriptProfiler(net::BidirectionalObjectTransport* transport);

	virtual void addListener(IListener* listener) override final;

	virtual void removeListener(IListener* listener) override final;

	void notifyListeners(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	AlignedVector< IListener* > m_listeners;
};

}
