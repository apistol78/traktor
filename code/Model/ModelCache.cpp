#include "Core/Io/FileSystem.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Thread/Acquire.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"

namespace traktor
{
	namespace model
	{
		namespace
		{

uint32_t hash(const std::wstring& text)
{
	Murmur3 cs;
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
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Ref< File > cachedFile = FileSystem::getInstance().get(cachedFileName);
		if (cachedFile != nullptr && file->getLastWriteTime() <= cachedFile->getLastWriteTime())
			haveCachedFile = true;
	}
	if (haveCachedFile)
	{
		// Valid cache entry found; read from model from cache,
		// do not use filter as it's written into cache after filter has been applied.
		return ModelFormat::readAny(cachedFileName, L"");
	}

	// No cached file exist; need to read source model.
	Ref< Model > model = ModelFormat::readAny(fileName, filter);
	if (!model)
		return nullptr;

	// Write cached copy of post-operation model.
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		Path intermediateFileName = cachedFileName.getPathNameNoExtension() + L"~." + cachedFileName.getExtension();

		if (!FileSystem::getInstance().makeAllDirectories(cachedFileName.getPathOnly()))
			return nullptr;

		if (ModelFormat::writeAny(intermediateFileName, model))
			FileSystem::getInstance().move(cachedFileName, intermediateFileName, true);
		else
			return nullptr;
	}

	// Return model; should be same as one we've written to cache.
	return model;
}

	}
}