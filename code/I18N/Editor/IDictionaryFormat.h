/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Object.h"
#include "Core/Ref.h"

namespace traktor
{

class IStream;
class Path;

}

namespace traktor::i18n
{

class Dictionary;

class IDictionaryFormat : public Object
{
	T_RTTI_CLASS;

public:
	virtual bool supportExtension(const std::wstring& extension) const = 0;

	virtual Ref< Dictionary > read(IStream* stream, int32_t keyColumn, int32_t textColumn) const = 0;

	virtual bool write(IStream* stream, const Dictionary* dictionary) const = 0;

	static Ref< Dictionary > readAny(const Path& filePath, int32_t keyColumn, int32_t textColumn);

	static Ref< Dictionary > readAny(IStream* stream, const std::wstring& extension, int32_t keyColumn, int32_t textColumn);

	static bool writeAny(const Path& filePath, const Dictionary* dictionary);

	static bool writeAny(IStream* stream, const std::wstring& extension, const Dictionary* dictionary);
};

}
