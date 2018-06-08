/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
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

bool NoFileStore::locked(const Path& filePath)
{
	return false;
}

bool NoFileStore::pending(const Path& filePath)
{
	return false;
}

bool NoFileStore::add(const Path& filePath)
{
	return true;
}

bool NoFileStore::remove(const Path& filePath)
{
	return FileSystem::getInstance().remove(filePath);
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
