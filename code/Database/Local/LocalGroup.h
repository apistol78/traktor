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
#include "Core/Io/Path.h"

namespace traktor::db
{

class Context;

/*! Local group.
 * \ingroup Database
 */
class LocalGroup : public IProviderGroup
{
	T_RTTI_CLASS;

public:
	explicit LocalGroup(Context& contex, const Path& groupPath, uint32_t flags);

	virtual std::wstring getName() const override final;

	virtual uint32_t getFlags() const override final;

	virtual bool rename(const std::wstring& name) override final;

	virtual bool remove() override final;

	virtual Ref< IProviderGroup > createGroup(const std::wstring& groupName) override final;

	virtual Ref< IProviderInstance > createInstance(const std::wstring& instanceName, const Guid& instanceGuid) override final;

	virtual bool getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances) override final;

private:
	Context& m_context;
	Path m_groupPath;
	uint32_t m_flags;
};

}
