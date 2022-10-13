#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Timer/Timer.h"
#include "SolutionBuilder/Project.h"
#include "SolutionBuilder/ScriptProcessor.h"
#include "SolutionBuilder/Solution.h"
#include "SolutionBuilder/Make/SolutionBuilderMake2.h"

namespace traktor
{
	namespace sb
	{

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderMake2", SolutionBuilderMake2, SolutionBuilder)

SolutionBuilderMake2::~SolutionBuilderMake2()
{
	safeDestroy(m_scriptProcessor);
}

bool SolutionBuilderMake2::create(const CommandLine& cmdLine)
{
	if (cmdLine.hasOption('s', L"make-solution-template"))
		m_solutionTemplate = cmdLine.getOption('s', L"make-solution-template").getString();
	if (cmdLine.hasOption('p', L"make-project-template"))
		m_projectTemplate = cmdLine.getOption('p', L"make-project-template").getString();

	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create(cmdLine))
		return false;

	return true;
}

bool SolutionBuilderMake2::generate(Solution* solution)
{
	AlignedVector< uint8_t > buf;
	Timer timer;

	// Create root path.
	if (!FileSystem::getInstance().makeAllDirectories(solution->getRootPath()))
		return false;

	log::info << L"Generating project makefiles..." << Endl;
	log::info << IncreaseIndent;

	if (!m_scriptProcessor->prepare(m_projectTemplate))
		return false;

	for (auto project : solution->getProjects())
	{
		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		const std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
		log::info << projectPath + L"/makefile";

		const double timeStart = timer.getElapsedTime();

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		// Generate project makefile.
		std::wstring projectOut;
		if (!m_scriptProcessor->generate(solution, project, L"", projectPath, projectOut))
			return false;

		Ref< IStream > file = FileSystem::getInstance().open(
			projectPath + L"/makefile",
			File::FmWrite
		);
		if (!file)
			return false;

		buf.resize(IEncoding::MaxEncodingSize * projectOut.length());
		const int32_t nbuf = Utf8Encoding().translate(projectOut.c_str(), (int)projectOut.length(), buf.ptr());
		file->write(buf.c_ptr(), nbuf);

		file->close();

		const double timeEnd = timer.getElapsedTime();
		log::info << L" (" << int32_t((timeEnd - timeStart) * 1000.0 + 0.5) << L" ms, " << nbuf << L" bytes)" << Endl;
	}

	log::info << DecreaseIndent;
	log::info << Endl;

	log::info << L"Generating solution makefile..." << Endl;
	log::info << IncreaseIndent;

	if (!m_scriptProcessor->prepare(m_solutionTemplate))
		return false;

	// Generate solution makefile.
	{
		std::wstring cprojectOut;
		if (!m_scriptProcessor->generate(solution, nullptr, L"", solution->getRootPath(), cprojectOut))
			return false;

		const std::wstring solutionPath = solution->getRootPath() + L"/" + solution->getName() + L".mak";
		log::info << solutionPath;

		const double timeStart = timer.getElapsedTime();

		Ref< IStream > file = FileSystem::getInstance().open(
			solutionPath,
			File::FmWrite
		);
		if (!file)
			return false;

		buf.resize(IEncoding::MaxEncodingSize * cprojectOut.length());
		const int32_t nbuf = Utf8Encoding().translate(cprojectOut.c_str(), (int)cprojectOut.length(), buf.ptr());
		file->write(buf.c_ptr(), nbuf);

		file->close();

		const double timeEnd = timer.getElapsedTime();
		log::info << L" (" << int32_t((timeEnd - timeStart) * 1000.0 + 0.5) << L" ms)" << Endl;
	}

	log::info << DecreaseIndent;
	log::info << Endl;

	return true;
}

void SolutionBuilderMake2::showOptions() const
{
	log::info << L"\t-s,-make-solution-template=[solution template file]" << Endl;
	log::info << L"\t-p,-make-project-template=[project template file]" << Endl;
}

	}
}
