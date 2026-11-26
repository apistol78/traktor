/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Heightfield/Editor/HeightfieldCache.h"

#include "Compress/Lzf/DeflateStreamLzf.h"
#include "Compress/Lzf/InflateStreamLzf.h"
#include "Core/Guid.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Reader.h"
#include "Core/Io/Writer.h"
#include "Core/Log/Log.h"
#include "Core/Singleton/SingletonManager.h"
#include "Core/Thread/Acquire.h"
#include "Heightfield/Heightfield.h"
#include "Heightfield/HeightfieldFormat.h"

namespace traktor::hf
{
namespace
{

const int32_t c_cacheVersion = 1;

HeightfieldCache* createInstance()
{
	HeightfieldCache* instance = new HeightfieldCache();
	SingletonManager::getInstance().add(instance);
	return instance;
}

}

HeightfieldCache& HeightfieldCache::getInstance()
{
	// Initialized once even when first used by several jobs at once.
	static HeightfieldCache* s_instance = createInstance();
	return *s_instance;
}

void HeightfieldCache::destroy()
{
	delete this;
}

Ref< Heightfield > HeightfieldCache::get(const Path& cachePath, const std::wstring& name, uint64_t stamp, const Vector4& worldExtent, const Vector4& worldCenter, const std::function< Ref< Heightfield >() >& create)
{
	if (cachePath.empty())
		return create();

	const Path fileName = cachePath.getPathName() + L"/" + name + L".tile";

	Semaphore* lock;
	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
		lock = &m_locks[fileName.getPathName()];
	}

	// Concurrent requests of same tile wait for first to read or create it.
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(*lock);

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead | File::FmMapped);
	if (file)
	{
		int32_t version = 0;
		uint64_t fileStamp = 0;
		Reader(file) >> version;
		Reader(file) >> fileStamp;

		Ref< Heightfield > tile;
		if (version == c_cacheVersion && fileStamp == stamp)
		{
			Ref< IStream > inflate = new compress::InflateStreamLzf(file);
			tile = HeightfieldFormat().read(inflate, worldExtent, worldCenter);
			inflate->close();
		}

		file->close();
		if (tile)
			return tile;
	}

	Ref< Heightfield > tile = create();
	if (!tile)
		return nullptr;

	// Written as intermediate file first so no reader, of any process, see a partial tile.
	const Path intermediateFileName = fileName.getPathName() + L"~" + Guid::create().format();
	Ref< IStream > out = FileSystem::getInstance().makeAllDirectories(fileName.getPathOnly()) ? FileSystem::getInstance().open(intermediateFileName, File::FmWrite) : nullptr;
	if (!out)
	{
		log::warning << L"Unable to write heightfield tile into cache \"" << fileName.getPathName() << L"\"." << Endl;
		return tile;
	}

	Writer(out) << c_cacheVersion;
	Writer(out) << stamp;

	Ref< IStream > deflate = new compress::DeflateStreamLzf(out);
	const bool written = HeightfieldFormat().write(deflate, tile);
	deflate->close();
	out->close();

	if (!written || !FileSystem::getInstance().move(fileName, intermediateFileName, true))
		FileSystem::getInstance().remove(intermediateFileName);

	return tile;
}

}
