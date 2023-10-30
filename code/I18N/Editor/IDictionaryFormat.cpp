/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileSystem.h"
#include "I18N/Editor/IDictionaryFormat.h"

namespace traktor::i18n
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.IDictionaryFormat", IDictionaryFormat, Object)

Ref< Dictionary > IDictionaryFormat::readAny(const Path& filePath, int32_t keyColumn, int32_t textColumn)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (file)
		return readAny(file, filePath.getExtension(), keyColumn, textColumn);
	else
		return nullptr;
}

Ref< Dictionary > IDictionaryFormat::readAny(IStream* stream, const std::wstring& extension, int32_t keyColumn, int32_t textColumn)
{
	Ref< Dictionary > dictionary;
	for (auto formatType : type_of< IDictionaryFormat >().findAllOf(false))
	{
		Ref< IDictionaryFormat > dictionaryFormat = dynamic_type_cast< IDictionaryFormat* >(formatType->createInstance());
		if (!dictionaryFormat)
			continue;

		if (!dictionaryFormat->supportExtension(extension))
			continue;

		dictionary = dictionaryFormat->read(stream, keyColumn, textColumn);
		if (dictionary)
			break;
	}
	return dictionary;
}

bool IDictionaryFormat::writeAny(const Path& filePath, const Dictionary* dictionary)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmWrite);
	if (file)
		return writeAny(file, filePath.getExtension(), dictionary);
	else
		return false;
}

bool IDictionaryFormat::writeAny(IStream* stream, const std::wstring& extension, const Dictionary* dictionary)
{
	for (auto formatType : type_of< IDictionaryFormat >().findAllOf())
	{
		Ref< IDictionaryFormat > dictionaryFormat = dynamic_type_cast< IDictionaryFormat* >(formatType->createInstance());
		if (!dictionaryFormat)
			continue;

		if (!dictionaryFormat->supportExtension(extension))
			continue;

		return dictionaryFormat->write(stream, dictionary);
	}
	return false;
}

}
