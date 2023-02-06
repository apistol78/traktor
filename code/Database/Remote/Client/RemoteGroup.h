/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Provider/IProviderGroup.h"

namespace traktor::db
{

class RemoteConnection;

/*! Remote group.
 * \ingroup Database
 */
class RemoteGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	explicit RemoteGroup(RemoteConnection* connection, uint32_t handle);

	virtual ~RemoteGroup();

	virtual std::wstring getName() const override final;

	virtual bool rename(const std::wstring& name) override final;

	virtual bool remove() override final;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) override final;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) override final;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) override final;

private:
	Ref< RemoteConnection > m_connection;
	uint32_t m_handle;
};

}
