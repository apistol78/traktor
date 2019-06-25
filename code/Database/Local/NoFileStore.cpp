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
