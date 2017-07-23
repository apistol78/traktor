/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Io/FileSystem.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/IStream.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Misc/CommandLine.h>
#include <Core/Log/Log.h>
#include "Source.h"

// Verify conditions
#include "Conditions/SingleEmptyLine.h"
#include "Conditions/Indentation.h"
#include "Conditions/IncludeGuard.h"
#include "Conditions/IncludeOrder.h"
#include "Conditions/Macros.h"
#include "Conditions/StatementDisposition.h"
#include "Conditions/ClassName.h"
#include "Conditions/Casts.h"

using namespace traktor;

namespace
{

class ReportLogTarget : public ILogTarget
{
public:
	ReportLogTarget(OutputStream* report)
	:	m_issues(0)
	,	m_report(report)
	{
	}

	virtual void log(const std::wstring& str)
	{
		log::error << str << Endl;
		++m_issues;

		if (m_report)
			*m_report << L"(" << m_issues << L") " << m_fileName.getPathName() << L" : " << str << Endl;
	}

	void setFile(const Path& fileName) { m_fileName = fileName; }

	uint32_t getIssues() const { return m_issues; }

private:
	uint32_t m_issues;
	Path m_fileName;
	Ref< OutputStream > m_report;
};

void verify(ReportLogTarget& report, const Path& pathName)
{
	Ref< File > file = FileSystem::getInstance().get(pathName);
	if (!file)
	{
		log::error << L"Unable to open \"" << pathName.getPathName() << L"\"; no such file or directory" << Endl;
		return;
	}

	if (!file->isDirectory())
	{
		Ref< IStream > file = FileSystem::getInstance().open(pathName, File::FmRead);
		if (!file)
		{
			log::error << L"Unable to open file \"" << pathName.getPathName() << L"\"" << Endl;
			return;
		}

		log::info << pathName.getFileName() << L"..." << Endl;
		log::info << IncreaseIndent;

		Source source;
		if (source.create(file))
		{
			bool isHeader =
				bool(compareIgnoreCase< std::wstring >(pathName.getExtension(), L"h") == 0) ||
				bool(compareIgnoreCase< std::wstring >(pathName.getExtension(), L"hpp") == 0);

			report.setFile(pathName);
			LogStream reportLog(&report);

			SingleEmptyLine().check(pathName, source, isHeader, reportLog);
			Indentation().check(pathName, source, isHeader, reportLog);
			IncludeGuard().check(pathName, source, isHeader, reportLog);
			IncludeOrder().check(pathName, source, isHeader, reportLog);
			Macros().check(pathName, source, isHeader, reportLog);
			StatementDisposition().check(pathName, source, isHeader, reportLog);
			ClassName().check(pathName, source, isHeader, reportLog);
			Casts().check(pathName, source, isHeader, reportLog);
		}
		else
			log::error << L"Unable to read file \"" << pathName.getPathName() << L"\"" << Endl;

		log::info << DecreaseIndent;

		file->close();
		file = 0;
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
						bool(compareIgnoreCase< std::wstring >(path.getExtension(), L"c") == 0) ||
						bool(compareIgnoreCase< std::wstring >(path.getExtension(), L"cpp") == 0) ||
						bool(compareIgnoreCase< std::wstring >(path.getExtension(), L"h") == 0) ||
						bool(compareIgnoreCase< std::wstring >(path.getExtension(), L"hpp") == 0);

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
		log::info << L"Usage: SourceVerify [path] (-r=report file)" << Endl;
		return 0;
	}

	Ref< FileOutputStream > fos;

	if (cmdLine.hasOption('r'))
	{
		std::wstring logFile = cmdLine.getOption('r').getString();
		Ref< IStream > file = FileSystem::getInstance().open(logFile, File::FmWrite);
		if (file)
			fos = new FileOutputStream(file, new Utf8Encoding());
	}

	ReportLogTarget report(fos);
	verify(report, cmdLine.getString(0));

	log::info << report.getIssues() << L" issue(s) found" << Endl;

	if (fos)
		fos->close();
}
