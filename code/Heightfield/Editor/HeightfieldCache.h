/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/Path.h"
#include "Core/Math/Vector4.h"
#include "Core/Ref.h"
#include "Core/Singleton/ISingleton.h"
#include "Core/Thread/Semaphore.h"

#include <functional>
#include <map>
#include <string>

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_HEIGHTFIELD_EDITOR_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::hf
{

class Heightfield;

/*! Cache of heightfield tiles evaluated by editor, such as outputs of layers.
 * \ingroup Heightfield
 *
 * Each tile is a file in cache directory, outside of source database, along with
 * stamp of what it was evaluated from; tile is evaluated again when stamp differ.
 */
class T_DLLCLASS HeightfieldCache : public ISingleton
{
public:
	static HeightfieldCache& getInstance();

	virtual void destroy() override final;

	/*! Get tile from cache, or create tile and write it into cache.
	 * \param cachePath Cache directory; nothing is cached if empty.
	 * \param name Name of tile, unique within cache directory.
	 * \param stamp Stamp of everything tile is evaluated from.
	 * \param create Create tile when not cached.
	 * \return Tile positioned in world, null if neither cached nor created.
	 */
	Ref< Heightfield > get(const Path& cachePath, const std::wstring& name, uint64_t stamp, const Vector4& worldExtent, const Vector4& worldCenter, const std::function< Ref< Heightfield >() >& create);

private:
	Semaphore m_lock;
	std::map< std::wstring, Semaphore > m_locks;
};

}
