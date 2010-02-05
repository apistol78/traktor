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

bool DefaultFileStore::add(const Path& filePath)
{
	// Cannot add already existing file.
	if (FileSystem::getInstance().exist(filePath))
		return false;

	return true;
}

bool DefaultFileStore::remove(const Path& filePath)
{
	Path filePathAlt = filePath.getPathName() + L"~";
	return FileSystem::getInstance().move(filePathAlt, filePath, true);
}

bool DefaultFileStore::edit(const Path& filePath)
{
	// Ensure file is not read-only; cannot edit those.
	Ref< File > file = FileSystem::getInstance().get(filePath);
	if (!file || file->isReadOnly())
		return false;

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
