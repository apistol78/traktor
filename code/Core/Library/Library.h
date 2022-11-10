/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <string>
#include <vector>
#include "Core/Object.h"
#include "Core/Io/Path.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_CORE_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

/*! System dynamic library.
 * \ingroup Core
 */
class T_DLLCLASS Library : public Object
{
	T_RTTI_CLASS;

public:
	/*! Open library.
	 *
	 * \param libraryName Name of library.
	 * \return True if library opened successfully.
	 */
	bool open(const Path& libraryName);

	/*! Open library.
	 *
	 * This method take user defined search paths
	 * which is used recursively, i.e. dependent
	 * libraries are also searched through the same
	 * paths.
	 *
	 * \param libraryName Name of library.
	 * \param searchPaths Custom search paths.
	 * \param includeDefaultPaths Include default search paths.
	 * \return True if library opened successfully.
	 */
	bool open(const Path& libraryName, const std::vector< Path >& searchPaths, bool includeDefaultPaths);

	/*! Close library. */
	void close();

	/*! Detach library.
	 *
	 * \note
	 * A detached library is not unloaded from the
	 * process until the process terminates.
	 */
	void detach();

	/*! Find exported symbol from library.
	 *
	 * \param symbol Name of exported symbol.
	 * \return Pointer to symbol.
	 */
	void* find(const std::wstring& symbol);

	/*! Get library path.
	 *
	 * \return Path to library.
	 */
	Path getPath() const;

private:
	void* m_handle = nullptr;
};

}

