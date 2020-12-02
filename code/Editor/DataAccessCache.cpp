#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Singleton/SingletonManager.h"
#include "Editor/DataAccessCache.h"

namespace traktor
{
	namespace editor
	{

DataAccessCache& DataAccessCache::getInstance()
{
	static DataAccessCache* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new DataAccessCache();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

bool DataAccessCache::create(const PropertyGroup* settings)
{
	m_cachePath = settings->getProperty< std::wstring >(L"Editor.DataAccessCachePath", L"data/Temp/DataCache");
	FileSystem::getInstance().makeAllDirectories(m_cachePath);
	return true;
}

void DataAccessCache::destroy() 
{
	// Flush pending writes.
}

Ref< Object > DataAccessCache::readObject(
	uint32_t key,
	const fn_readObject_t& read,
	const fn_writeObject_t& write,
	const fn_createObject_t& create
)
{
	// Get object from object table.

	Path fileName = m_cachePath.getPathName() + L"/" + str(L"%08x.blob", key);

	Ref< IStream > blobStream = FileSystem::getInstance().open(fileName, File::FmRead);
	if (blobStream)
	{
		Ref< Object > object = read(blobStream);
		// Add to object table.
		return object;
	}

	// No cached entry.
	Ref< Object > object = create();
	if (!object)
		return nullptr;

	// Add to object table.

	// Write to physical cache.
	// \todo Do this on a writer thread.
	blobStream = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (blobStream)
	{
		write(object, blobStream);
		safeClose(blobStream);
	}

	return object;
}

	}
}
