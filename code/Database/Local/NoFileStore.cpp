/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Database/Types.h"
#include "Database/Local/NoFileStore.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.NoFileStore", 0, NoFileStore, IFileStore)

bool NoFileStore::create(const ConnectionString& connectionString)
{
	return true;
}

void NoFileStore::destroy()
{
}

uint32_t NoFileStore::flags(const Path& filePath)
{
	Ref< File > file = FileSystem::getInstance().get(filePath);
	if (file)
	{
		if ((file->getFlags() & File::FfReadOnly) != 0)
			return IfReadOnly;
		else
			return IfNormal;
	}
	return IfNormal;
}

bool NoFileStore::add(const Path& filePath)
{
	return true;
}

bool NoFileStore::remove(const Path& filePath)
{
	// Try to remove file, if remove fails then we assume file didn't exist thus
	// file store's remove succeeded.
	FileSystem::getInstance().remove(filePath);
	return true;
}

bool NoFileStore::edit(const Path& filePath)
{
	return true;
}

bool NoFileStore::rollback(const Path& filePath)
{
	return false;
}

bool NoFileStore::clean(const Path& filePath)
{
	return true;
}

	}
}
