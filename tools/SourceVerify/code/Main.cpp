#include <Core/Io/FileSystem.h>
#include <Core/Io/Stream.h>
#include <Core/Io/StringReader.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>

// Verify conditions
#include "Conditions/SingleEmptyLine.h"
#include "Conditions/Indentation.h"
#include "Conditions/IncludeGuard.h"
#include "Conditions/IncludeOrder.h"
#include "Conditions/Macros.h"

using namespace traktor;

namespace
{

class ReportLogTarget : public LogTarget
{
public:
	ReportLogTarget()
	:	m_issues(0)
	{
	}

	virtual void log(const std::wstring& str)
	{
		log::error << str << Endl;
		++m_issues;
	}

	uint32_t getIssues() const { return m_issues; }

private:
	uint32_t m_issues;
};

void verify(OutputStream& report, const Path& pathName)
{
	Ref< File > file = FileSystem::getInstance().get(pathName);
	if (!file)
	{
		log::error << L"Unable to open \"" << pathName << L"\"; no such file or directory" << Endl;
		return;
	}

	if (!file->isDirectory())
	{
		Ref< Stream > file = FileSystem::getInstance().open(pathName, File::FmRead);
		if (!file)
		{
			log::error << L"Unable to open file \"" << pathName << L"\"" << Endl;
			return;
		}

		log::info << pathName.getFileName() << L"..." << Endl;
		log::info << IncreaseIndent;

		StringReader sr(file, gc_new< Utf8Encoding >());

		std::vector< std::wstring > lines;
		std::wstring line;

		while (sr.readLine(line) >= 0)
			lines.push_back(line);

		file->close();
		file = 0;

		bool isHeader =
			bool(compareIgnoreCase(pathName.getExtension(), L"h") == 0) ||
			bool(compareIgnoreCase(pathName.getExtension(), L"hpp") == 0);

		SingleEmptyLine().check(lines, isHeader, report);
		Indentation().check(lines, isHeader, report);
		IncludeGuard().check(lines, isHeader, report);
		IncludeOrder().check(lines, isHeader, report);
		Macros().check(lines, isHeader, report);

		log::info << DecreaseIndent;
	}
	else
	{
		RefArray< File > files;
		FileSystem::getInstance().find(Path(pathName.getPathName() + L"/*.*"), files);

		if (!files.empty())
		{
			log::info << L"Enter " << pathName.getFileName() << L"..." << Endl;
			log::info << IncreaseIndent;

			for (RefArray< File >::iterator i = files.begin(); i != files.end(); ++i)
			{
				Path path = (*i)->getPath();
				if ((*i)->isDirectory())
				{
					if (path.getFileName() != L"." && path.getFileName() != L"..")
						verify(report, path);
				}
				else
				{
					bool isSource =
						bool(compareIgnoreCase(path.getExtension(), L"c") == 0) ||
						bool(compareIgnoreCase(path.getExtension(), L"cpp") == 0) ||
						bool(compareIgnoreCase(path.getExtension(), L"h") == 0) ||
						bool(compareIgnoreCase(path.getExtension(), L"hpp") == 0);

					if (isSource)
						verify(report, path);
				}
			}

			log::info << DecreaseIndent;
		}
	}
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
	{
		log::info << L"Usage: SourceVerify [path]" << Endl;
		return 0;
	}

	ReportLogTarget target;
	LogStream report(&target);

	verify(report, cmdLine.getString(0));

	log::info << target.getIssues() << L" issue(s) found" << Endl;
}
