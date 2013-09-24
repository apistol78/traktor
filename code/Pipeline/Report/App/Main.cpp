#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Pipeline/Report/App/ScriptProcessor.h"

using namespace traktor;

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 1)
		return 1;

	editor::ScriptProcessor filter;
	if (!filter.create())
	{
		log::error << L"Failed to create script processor" << Endl;
		return 2;
	}

	std::wstring page;
	if (!filter.generateFromFile(cmdLine.getString(0), page))
	{
		log::error << L"Failed to generate report from " << cmdLine.getString(0) << Endl;
		return 3;
	}

	log::info << page << Endl;

	filter.destroy();
}
