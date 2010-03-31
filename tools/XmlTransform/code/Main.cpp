#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Xml/Document.h>
#include "Transformer.h"

using namespace traktor;

void recursiveFindFiles(const Path& currentPath, std::set< Path >& outFiles)
{
	RefArray< File > localFiles;
	FileSystem::getInstance().find(currentPath, localFiles);

	for (RefArray< File >::iterator i = localFiles.begin(); i != localFiles.end(); ++i)
	{
		if ((*i)->isReadOnly())
			continue;

		Path filePath = (*i)->getPath();
		if (filePath.getFileName() == L"." || filePath.getFileName() == L"..")
			continue;

		if (!(*i)->isDirectory())
			outFiles.insert(filePath);
		else
			recursiveFindFiles(filePath, outFiles);
	}
}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 2)
	{
		traktor::log::info << L"XmlTransform v1.0" << Endl;
		traktor::log::info << Endl;
		traktor::log::info << L"Scriptable XML batch transformation tool" << Endl;
		traktor::log::info << Endl;
		traktor::log::info << L"XmlTransform (options) script file(s)" << Endl;
		traktor::log::info << Endl;
		traktor::log::info << L"Options:" << Endl;
		traktor::log::info << L"\t-r\tRecursive" << Endl;
		traktor::log::info << L"\t-s\tAppend suffix to result file" << Endl;
		return -1;
	}

	Ref< IStream > file = FileSystem::getInstance().open(cmdLine.getString(0), File::FmRead);
	if (!file)
	{
		traktor::log::error << L"Unable to read script; no such file" << Endl;
		return 1;
	}

	Utf8Encoding encoding;
	StringReader reader(file, &encoding);
	StringOutputStream ss;
	std::wstring tmp;

	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	file->close();

	Transformer transformer;
	if (!transformer.create(ss.str()))
	{
		traktor::log::error << L"Unable to initialize transformer" << Endl;
		return 2;
	}

	std::set< Path > files;
	for (int i = 1; i < cmdLine.getCount(); ++i)
		recursiveFindFiles(cmdLine.getString(i), files);

	traktor::log::info << L"Found " << files.size() << L" file(s)" << Endl;

	for (std::set< Path >::iterator i = files.begin(); i != files.end(); ++i)
	{
		xml::Document document;
		if (!document.loadFromFile(*i))
		{
			traktor::log::error << L"Unable to parse \"" << i->getPathName() << L"\"" << Endl;
			continue;
		}

		traktor::log::info << L"Transforming \"" << i->getPathName() << L"\"..." << Endl;
		int32_t changes = transformer.transform(&document);
		if (changes)
		{
			traktor::log::info << L"\t" << changes << L" change(s); updating file..." << Endl;

			std::wstring outputFile = i->getPathName();
			if (cmdLine.hasOption('s'))
				outputFile += cmdLine.getOption('s').getString();

			if (!document.saveAsFile(outputFile))
				traktor::log::error << L"Unable to save \"" << outputFile << L"\"" << Endl;
		}
	}

	traktor::log::info << L"Finished" << Endl;
	return 0;
}
