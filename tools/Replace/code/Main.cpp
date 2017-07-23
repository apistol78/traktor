/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <list>
#include <Core/Io/AnsiEncoding.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Io/Utf16Encoding.h>
#include <Core/Io/Utf32Encoding.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	std::wstring fileName;
	std::wstring encodingName;
	std::list< std::pair< std::wstring, std::wstring > > subst;

	if (cmdLine.getCount() < 3)
	{
		log::info << L"Usage: Replace -e|--encoding=(encoding) (file) ((needle) (substitute))..." << Endl;
		log::info << L"       Encodings: utf8, utf16, utf32, latin1" << Endl;
		return 0;
	}

	fileName = cmdLine.getString(0);

	if (cmdLine.hasOption('e', L"encoding"))
		encodingName = cmdLine.getOption('e', L"encoding").getString();
	else
		encodingName = L"utf8";

	Ref< IEncoding > encoding;
	if (encodingName == L"utf8")
		encoding = new Utf8Encoding();
	else if (encodingName == L"utf16")
		encoding = new Utf16Encoding();
	else if (encodingName == L"utf32")
		encoding = new Utf32Encoding();
	else if (encodingName == L"latin1")
		encoding = new AnsiEncoding();

	if (!encoding)
	{
		log::error << L"Unknown encoding \"" << encodingName << L"\"" << Endl;
		return 1;
	}

	for (int32_t i = 1; i < cmdLine.getCount(); i += 2)
	{
		subst.push_back(std::make_pair(
			cmdLine.getString(i),
			cmdLine.getString(i + 1)
		));
	}

	Ref< IStream > file = FileSystem::getInstance().open(fileName, File::FmRead);
	if (!file)
	{
		log::error << L"Unable to open file \"" << fileName << L"\"" << Endl;
		return 2;
	}

	std::list< std::wstring > lines;

	StringReader sr(file, encoding);
	std::wstring line;
	while (sr.readLine(line) >= 0)
		lines.push_back(line);

	file->close();
	file = 0;

	log::info << L"Read " << int32_t(lines.size()) << L" line(s)" << Endl;
	
	uint32_t changeCount = 0, changedLineCount = 0;
	for (std::list< std::wstring >::iterator i = lines.begin(); i != lines.end(); ++i)
	{
		bool changedLine = false;
		for (std::list< std::pair< std::wstring, std::wstring > >::const_iterator j = subst.begin(); j != subst.end(); ++j)
		{
			std::wstring tmp = replaceAll(*i, j->first, j->second);
			if (tmp != *i)
			{
				*i = tmp;

				++changeCount;
				if (!changedLine)
				{
					++changedLineCount;
					changedLine = true;
				}
			}
		}
	}

	log::info << changeCount << L" change(s) performed on " << changedLineCount << L" line(s)" << Endl;

	file = FileSystem::getInstance().open(fileName, File::FmWrite);
	if (!file)
	{
		log::error << L"Unable to open file \"" << fileName << L"\"" << Endl;
		return 2;
	}

	FileOutputStream fos(file, encoding);
	for (std::list< std::wstring >::iterator i = lines.begin(); i != lines.end(); ++i)
		fos << *i << Endl;

	file->close();
	file = 0;

	return 0;
}
