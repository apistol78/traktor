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
#include "Core/Ref.h"
#include "Net/Discovery/IDiscoveryMessage.h"

namespace traktor::net
{

class IService;

class DmServiceInfo : public IDiscoveryMessage
{
	T_RTTI_CLASS;

public:
	DmServiceInfo() = default;

	explicit DmServiceInfo(const Guid& serviceGuid, IService* service);

	const Guid& getServiceGuid() const { return m_serviceGuid; }

	IService* getService() const { return m_service; }

	virtual void serialize(ISerializer& s) override final;

private:
	Guid m_serviceGuid;
	Ref< IService > m_service;
};

}
