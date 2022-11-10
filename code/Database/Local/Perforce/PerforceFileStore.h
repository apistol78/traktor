/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Local/IFileStore.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_DATABASE_LOCAL_P4_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace db
	{

class PerforceClient;
class PerforceChangeList;

/*! Perforce file store.
 * \ingroup Database
 */
class T_DLLCLASS PerforceFileStore : public IFileStore
{
	T_RTTI_CLASS;

public:
	virtual bool create(const ConnectionString& connectionString) override final;

	virtual void destroy() override final;

	virtual uint32_t flags(const Path& filePath) override final;

	virtual bool add(const Path& filePath) override final;

	virtual bool remove(const Path& filePath) override final;

	virtual bool edit(const Path& filePath) override final;

	virtual bool rollback(const Path& filePath) override final;

	virtual bool clean(const Path& filePath) override final;

private:
	Ref< PerforceClient > m_p4client;
	Ref< PerforceChangeList > m_p4changeList;
};

	}
}

