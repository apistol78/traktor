/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Io/IStream.h"
#include "Core/Io/Path.h"
#include "Json/JsonArray.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_JSON_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::json
{

/*! JSON document.
 * \ingroup JSON
 */
class T_DLLCLASS JsonDocument : public JsonArray
{
	T_RTTI_CLASS;

public:
	/*! Load JSON document from file.
	 *
	 * \param fileName Path to file.
	 * \return True if successfully loaded.
	 */
	bool loadFromFile(const Path& fileName);

	/*! Load JSON document from stream.
	 *
	 * \param stream Stream to load from.
	 * \return True if successfully loaded.
	 */
	bool loadFromStream(IStream* stream);

	/*! Load JSON document from text string.
	 *
	 * \param text Text string.
	 * \return True if successfully loaded.
	 */
	bool loadFromText(const std::wstring& text);

	/*! Save JSON document to file.
	 *
	 * \param fileName Path to file.
	 * \return True if successfully saved.
	 */
	bool saveToFile(const Path& fileName);

	/*! Save JSON document to stream.
	 *
	 * \param stream Stream to save into.
	 * \return True if successfully saved.
	 */
	bool saveToStream(IStream* stream);
};

}
