#include <Core/Io/FileSystem.h>
#include <Core/Io/IStream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/StringOutputStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Library/Library.h>
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include "ActiveHtmlFilter.h"

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
		return 1;

#if !defined(T_STATIC)
	Library().open(L"Traktor.Script.Js");
	Library().open(L"Traktor.Sql.Sqlite3");
#endif

	Ref< IStream > file = FileSystem::getInstance().open(cmdLine.getString(0), File::FmRead);
	if (!file)
		return 1;

	Utf8Encoding encoding;
	StringReader reader(file, &encoding);
	StringOutputStream ss;
	std::wstring tmp;

	while (reader.readLine(tmp) >= 0)
		ss << tmp << Endl;

	file->close();

	ActiveHtmlFilter filter;
	if (!filter.create())
		return 1;

	std::wstring page = filter.generate(ss.str());
	traktor::log::info << page;

	filter.destroy();
}
