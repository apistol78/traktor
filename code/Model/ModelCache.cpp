#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Adler32.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/Mutex.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

const Guid c_globalLockId(L"{BF9144AA-FA5C-49D6-8E5B-9593EEE92148}");
static Mutex s_lock(c_globalLockId);

uint32_t hash(const std::wstring& text)
{
	Adler32 cs;
	cs.begin();
	cs.feed(text);
	cs.end();
	return cs.get();
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.model.ModelCache", ModelCache, Object)

ModelCache::ModelCache(const Path& cachePath)
:	m_cachePath(cachePath)
{
}

Ref< Model > ModelCache::get(const Path& fileName, const std::wstring& filter)
{
	// Get information about source file.
	Ref< File > file = FileSystem::getInstance().get(fileName);
	if (!file)
		return nullptr;

	// Calculate hash of resolved file name.
	uint32_t fileNameHash = hash(fileName.getPathName() + L"!" + filter);

	// Generate file name of cached model.
	Path cachedFileName = m_cachePath.getPathName() + L"/" + toString(fileNameHash) + L".tmd";

	// Check if cached file exist and if it's time stamp match source file's.
	bool haveCachedFile = false;
	{
		T_ANONYMOUS_VAR(Acquire< Mutex >)(s_lock);
		Ref< File > cachedFile = FileSystem::getInstance().get(cachedFileName);
		if (cachedFile != nullptr && file->getLastWriteTime() <= cachedFile->getLastWriteTime())
			haveCachedFile = true;
	}
	if (haveCachedFile)
	{
		log::info << L"READ MODEL " << cachedFileName.getPathName() << Endl;

		// Valid cache entry found; read from model from cache,
		// do not use filter as it's written into cache after filter has been applied.
		return ModelFormat::readAny(cachedFileName, L"");
	}

	// No cached file exist; need to read source model and apply all operations.
	Ref< Model > model = ModelFormat::readAny(fileName, filter);
	if (!model)
		return nullptr;

	if (model)
		log::info << L"READ MODEL " << fileName.getPathName() << L", CACHE AS " << cachedFileName.getPathName() << Endl;

	// Write cached copy of post-operation model.
	{
		T_ANONYMOUS_VAR(Acquire< Mutex >)(s_lock);
		FileSystem::getInstance().makeAllDirectories(cachedFileName.getPathOnly());
		ModelFormat::writeAny(cachedFileName, model);
	}

	// Return model; should be same as one we've written to cache.
	return model;
}

	}
}