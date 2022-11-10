/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Guid.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/IDiscoveryMessage.h"

namespace traktor
{
	namespace net
	{

class DmFindServices : public IDiscoveryMessage
{
	T_RTTI_CLASS;

public:
	DmFindServices() = default;

	explicit DmFindServices(const Guid& managerGuid, const SocketAddressIPv4& replyTo);

	const Guid& getManagerGuid() const { return m_managerGuid; }

	const SocketAddressIPv4& getReplyTo() const { return m_replyTo; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_managerGuid;
	SocketAddressIPv4 m_replyTo;
};

	}
}

