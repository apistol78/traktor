/*
 * TRAKTOR
 * Copyright (c) 2022-2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "Core/Misc/Murmur3.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Model/Model.h"
#include "Model/ModelCache.h"
#include "Model/ModelFormat.h"

namespace traktor::model
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

ModelCache& ModelCache::getInstance()
{
	static ModelCache* s_instance = nullptr;
	if (!s_instance)
	{
		s_instance = new ModelCache();
		SingletonManager::getInstance().add(s_instance);
	}
	return *s_instance;
}

void ModelCache::destroy()
{
	delete this;
}

Ref< const Model > ModelCache::get(const Path& cachePath, const Path& fileName, const std::wstring& filter)
{
	const auto key = std::make_pair(fileName, filter);

	// Get information about source file.
	Ref< File > file = FileSystem::getInstance().get(fileName);
	if (!file)
		return nullptr;

	// First check if we have recent model loaded into memory.
	auto it = m_models.find(key);
	if (it != m_models.end())
	{
		if (it->second.timeStamp >= file->getLastWriteTime())
			return it->second.model;
	}

	// Calculate hash of resolved file name.
	const uint32_t fileNameHash = hash(fileName.getPathName() + L"!" + filter);

	// Generate file name of cached model.
	const Path cachedFileName = cachePath.getPathName() + L"/" + str(L"%08x.tmd", fileNameHash);

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
		Ref< const Model > model = ModelFormat::readAny(cachedFileName, L"");
		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
			m_models.insert(key, { model, file->getLastWriteTime() });
		}
		return model;
	}

	// No cached file exist; need to read source model.
	Ref< const Model > model = ModelFormat::readAny(fileName, filter);
	if (!model)
		return nullptr;

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

		// Add model to memory map.
		m_models.insert(key, { model, file->getLastWriteTime() });

		// Write cached copy of post-operation model.
		const Path intermediateFileName = cachedFileName.getPathNameNoExtension() + L"~." + cachedFileName.getExtension();

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

Ref< Model > ModelCache::getMutable(const Path& cachePath, const Path& fileName, const std::wstring& filter)
{
	Ref< const Model > model = get(cachePath, fileName, filter);
	return model != nullptr ? DeepClone(model).create< Model >() : nullptr;
}

}
