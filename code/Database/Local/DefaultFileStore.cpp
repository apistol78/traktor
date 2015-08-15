#include "Core/Io/FileSystem.h"
#include "Database/Local/DefaultFileStore.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.DefaultFileStore", DefaultFileStore, IFileStore)

bool DefaultFileStore::create(const ConnectionString& connectionString)
{
	return true;
}

void DefaultFileStore::destroy()
{
}

bool DefaultFileStore::pending(const Path& filePath)
{
	return false;
}

bool DefaultFileStore::add(const Path& filePath)
{
	if (FileSystem::getInstance().exist(filePath))
		return true;
	else
	{
		// File doesn't exist.
		return false;
	}
}

bool DefaultFileStore::remove(const Path& filePath)
{
	if (FileSystem::getInstance().exist(filePath))
	{
		Path filePathAlt = filePath.getPathName() + L"~";
		return FileSystem::getInstance().move(
			filePathAlt,
			filePath,
			true
		);
	}
	else
	{
		// File doesn't exist.
		return true;
	}
}

bool DefaultFileStore::edit(const Path& filePath)
{
	Ref< File > file = FileSystem::getInstance().get(filePath);
	if (!file)
	{
		// File doesn't exist; no need to take snapshot.
		return true;
	}

	if (file->isReadOnly())
	{
		// Read-only file; not allowed to edit those.
		return false;
	}

	Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().copy(filePathAlt, filePath, true);
}

bool DefaultFileStore::rollback(const Path& filePath)
{
	Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().move(filePath, filePathAlt, true);
}

bool DefaultFileStore::clean(const Path& filePath)
{
	Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().remove(filePathAlt);
}

	}
}
