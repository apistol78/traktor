/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileSystem.h"
#include "I18N/Editor/IDictionaryFormat.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.i18n.IDictionaryFormat", IDictionaryFormat, Object)

Ref< Dictionary > IDictionaryFormat::readAny(const Path& filePath, int32_t keyColumn, int32_t textColumn)
{
	Ref< IStream > file = FileSystem::getInstance().open(filePath, File::FmRead);
	if (file)
		return readAny(file, filePath.getExtension(), keyColumn, textColumn);
	else
		return 0;
}

Ref< Dictionary > IDictionaryFormat::readAny(IStream* stream, const std::wstring& extension, int32_t keyColumn, int32_t textColumn)
{
	Ref< Dictionary > dictionary;

	TypeInfoSet formatTypes;
	type_of< IDictionaryFormat >().findAllOf(formatTypes, false);

	for (TypeInfoSet::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< IDictionaryFormat > dictionaryFormat = dynamic_type_cast< IDictionaryFormat* >((*i)->createInstance());
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
	TypeInfoSet formatTypes;
	type_of< IDictionaryFormat >().findAllOf(formatTypes);

	for (TypeInfoSet::iterator i = formatTypes.begin(); i != formatTypes.end(); ++i)
	{
		Ref< IDictionaryFormat > dictionaryFormat = dynamic_type_cast< IDictionaryFormat* >((*i)->createInstance());
		if (!dictionaryFormat)
			continue;

		if (!dictionaryFormat->supportExtension(extension))
			continue;

		return dictionaryFormat->write(stream, dictionary);
	}

	return false;
}

	}
}
