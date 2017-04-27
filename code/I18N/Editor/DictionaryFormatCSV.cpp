/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/StringReader.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "I18N/Dictionary.h"
#include "I18N/Editor/DictionaryFormatCSV.h"

namespace traktor
{
	namespace i18n
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.i18n.DictionaryFormatCSV", 0, DictionaryFormatCSV, IDictionaryFormat)

bool DictionaryFormatCSV::supportExtension(const std::wstring& extension) const
{
	return compareIgnoreCase< std::wstring >(extension, L"csv") == 0;
}

Ref< Dictionary > DictionaryFormatCSV::read(IStream* stream, int32_t keyColumn, int32_t textColumn) const
{
	Ref< Dictionary > dictionary = new Dictionary();

	StringReader sr(stream, new Utf8Encoding());
	std::wstring line;

	while (sr.readLine(line) >= 0)
	{
		std::vector< std::wstring > columns;

		uint32_t i0 = 0;
		bool quote = false;

		for (uint32_t i = 0; i < line.length(); ++i)
		{
			if (line[i] == L',' && quote == false)
			{
				std::wstring tmp = trim(line.substr(i0, i - i0));
				if (tmp.length() >= 2 && tmp[0] == L'\"' && tmp[tmp.length() - 1] == L'\"')
					tmp = tmp.substr(1, tmp.length() - 2);
				columns.push_back(tmp);
				i0 = i + 1;
			}
			if (line[i] == L'\"')
			{
				quote = !quote;
			}
		}

		if (!quote)
		{
			std::wstring tmp = trim(line.substr(i0));
			if (tmp.length() >= 2 && tmp[0] == L'\"' && tmp[tmp.length() - 1] == L'\"')
				tmp = tmp.substr(1, tmp.length() - 2);
			columns.push_back(tmp);
		}

		Split< std::wstring >::any(line, L",", columns, true);
		if (columns.size() > std::max(keyColumn, textColumn) && !columns[keyColumn].empty())
		{
			dictionary->set(
				columns[keyColumn],
				columns[textColumn]
			);
		}
	}

	return dictionary;
}

bool DictionaryFormatCSV::write(IStream* stream, const Dictionary* dictionary) const
{
	FileOutputStream fos(stream, new Utf8Encoding());
	const std::map< std::wstring, std::wstring >& map = dictionary->get();
	for (std::map< std::wstring, std::wstring >::const_iterator i = map.begin(); i != map.end(); ++i)
		fos << i->first << L"," << i->second << Endl;
	return true;
}

	}
}
