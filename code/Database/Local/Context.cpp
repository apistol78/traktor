/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Local/Context.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.Context", Context, Object)

Context::Context(bool preferBinary, IFileStore* fileStore)
:	m_sessionGuid(Guid::create())
,	m_preferBinary(preferBinary)
,	m_fileStore(fileStore)
{
}

const Guid& Context::getSessionGuid() const
{
	return m_sessionGuid;
}

bool Context::preferBinary() const
{
	return m_preferBinary;
}

IFileStore* Context::getFileStore() const
{
	return m_fileStore;
}

	}
}
