#include <algorithm>
#include <Core/Io/DynamicMemoryStream.h>
#include <Core/Io/FileOutputStream.h>
#include <Core/Io/FileSystem.h>
#include <Core/Io/Utf8Encoding.h>
#include <Core/Log/Log.h>
#include "SolutionBuilderLIB/Configuration.h"
#include "SolutionBuilderLIB/ExternalDependency.h"
#include "SolutionBuilderLIB/File.h"
#include "SolutionBuilderLIB/Filter.h"
#include "SolutionBuilderLIB/Project.h"
#include "SolutionBuilderLIB/ProjectDependency.h"
#include "SolutionBuilderLIB/ScriptProcessor.h"
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/Eclipse/SolutionBuilderEclipse.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderEclipse", SolutionBuilderEclipse, SolutionBuilder)

SolutionBuilderEclipse::SolutionBuilderEclipse()
{
}

bool SolutionBuilderEclipse::create(const traktor::CommandLine& cmdLine)
{
	m_scriptProcessor = new ScriptProcessor();
	if (!m_scriptProcessor->create())
		return false;

	return true;
}

bool SolutionBuilderEclipse::generate(Solution* solution)
{
	// Create root path.
	if (!FileSystem::getInstance().makeDirectory(solution->getRootPath()))
		return false;

	const RefArray< Project >& projects = solution->getProjects();
	for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
	{
		const Project* project = *i;

		// Skip disabled projects.
		if (!project->getEnable())
			continue;

		const RefArray< ProjectItem >& items = project->getItems();

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
		std::wstring projectFileName = projectPath + L"/.project";
		std::wstring cprojectFileName = projectPath + L"/.cproject";

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		// Generate .project
		{
			std::wstring projectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, L"$(TRAKTOR_HOME)/bin/eclipse-mingw-project.sb", projectOut))
				return false;

			Ref< IStream > file = FileSystem::getInstance().open(
				projectFileName,
				traktor::File::FmWrite
			);
			if (!file)
				return false;

			FileOutputStream(file, new Utf8Encoding()) << projectOut;

			file->close();

			/*
			std::vector< uint8_t > buffer;
			buffer.reserve(40000);

			DynamicMemoryStream bufferStream(buffer, false, true);
			FileOutputStream os(&bufferStream, new Utf8Encoding());

			os << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << Endl;
			os << L"<projectDescription>" << Endl;
			os << IncreaseIndent;
			os << L"<name>" << project->getName() << L"</name>" << Endl;
			os << L"<comment/>" << Endl;

			const RefArray< Dependency >& dependencies = project->getDependencies();
			if (!dependencies.empty())
			{
				os << L"<projects>" << Endl;
				os << IncreaseIndent;

				for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
				{
					const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j);
					if (projectDependency)
					{
						const Project* projectDep = projectDependency->getProject();
						T_ASSERT (projectDep);
	
						os << L"<project>" << projectDep->getName() << L"</project>" << Endl;
					}
				}

				os << DecreaseIndent;
				os << L"</projects>" << Endl;
			}
			else
				os << L"<projects/>" << Endl;

			os << L"<buildSpec>" << Endl;
			os << IncreaseIndent;
			os << L"<buildCommand>" << Endl;
			os << IncreaseIndent;
			os << L"<name>org.eclipse.cdt.managedbuilder.core.genmakebuilder</name>" << Endl;
			os << L"<triggers>clean,full,incremental,</triggers>" << Endl;
			os << L"<arguments>" << Endl;
			os << IncreaseIndent;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>?name?</key>" << Endl;
			os << L"\t<value></value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.append_environment</key>" << Endl;
			os << L"\t<value>true</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.buildArguments</key>" << Endl;
			os << L"\t<value></value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.buildCommand</key>" << Endl;
			os << L"\t<value>make</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.buildLocation</key>" << Endl;
			os << L"\t<value>${workspace_loc:/" << project->getName() << L"/Debug}</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.contents</key>" << Endl;
			os << L"\t<value>org.eclipse.cdt.make.core.activeConfigSettings</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.enableAutoBuild</key>" << Endl;
			os << L"\t<value>false</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.enableCleanBuild</key>" << Endl;
			os << L"\t<value>true</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.enableFullBuild</key>" << Endl;
			os << L"\t<value>true</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.stopOnError</key>" << Endl;
			os << L"\t<value>true</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << L"<dictionary>" << Endl;
			os << L"\t<key>org.eclipse.cdt.make.core.useDefaultBuildCmd</key>" << Endl;
			os << L"\t<value>true</value>" << Endl;
			os << L"</dictionary>" << Endl;
			os << DecreaseIndent;
			os << L"</arguments>" << Endl;
			os << DecreaseIndent;
			os << L"</buildCommand>" << Endl;
			os << L"<buildCommand>" << Endl;
			os << IncreaseIndent;
			os << L"<name>org.eclipse.cdt.managedbuilder.core.ScannerConfigBuilder</name>" << Endl;
			os << L"<triggers>full,incremental,</triggers>" << Endl;
			os << L"<arguments/>" << Endl;
			os << DecreaseIndent;
			os << L"</buildCommand>" << Endl;
			os << DecreaseIndent;
			os << L"</buildSpec>" << Endl;

			os << L"<natures>" << Endl;
			os << IncreaseIndent;
			os << L"<nature>org.eclipse.cdt.core.cnature</nature>" << Endl;
			os << L"<nature>org.eclipse.cdt.core.ccnature</nature>" << Endl;
			os << L"<nature>org.eclipse.cdt.managedbuilder.core.managedBuildNature</nature>" << Endl;
			os << L"<nature>org.eclipse.cdt.managedbuilder.core.ScannerConfigNature</nature>" << Endl;
			os << DecreaseIndent;
			os << L"</natures>" << Endl;

			os << L"<linkedResources>" << Endl;
			os << IncreaseIndent;

			for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
				addItem(solution, project, *i, L"", os);

			os << DecreaseIndent;
			os << L"</linkedResources>" << Endl;

			os << DecreaseIndent;
			os << L"</projectDescription>" << Endl;

			os.close();

			if (!buffer.empty())
			{
				Ref< IStream > file = FileSystem::getInstance().open(
					projectFileName,
					traktor::File::FmWrite
				);
				if (!file)
					return false;
				file->write(&buffer[0], int(buffer.size()));
				file->close();
			}
			*/
		}

		// Generate .cproject
		{
			std::wstring cprojectOut;
			if (!m_scriptProcessor->generateFromFile(solution, project, projectPath, L"$(TRAKTOR_HOME)/bin/eclipse-mingw-cproject.sb", cprojectOut))
				return false;

			Ref< IStream > file = FileSystem::getInstance().open(
				cprojectFileName,
				traktor::File::FmWrite
			);
			if (!file)
				return false;

			FileOutputStream(file, new Utf8Encoding()) << cprojectOut;

			file->close();

			/*

			std::vector< uint8_t > buffer;
			buffer.reserve(40000);

			DynamicMemoryStream bufferStream(buffer, false, true);
			FileOutputStream os(&bufferStream, new Utf8Encoding());

			os << L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << Endl;
			os << L"<?fileVersion 4.0.0?>" << Endl;
			os << Endl;
			os << L"<cproject storage_type_id=\"org.eclipse.cdt.core.XmlProjectDescriptionStorage\">" << Endl;
			os << IncreaseIndent;

			os << L"<storageModule moduleId=\"org.eclipse.cdt.core.settings\">" << Endl;
			os << IncreaseIndent;

			const RefArray< Configuration >& configurations = project->getConfigurations();
			for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
			{
				if ((*j)->getTargetProfile() == Configuration::TpDebug)
					os << L"<cconfiguration id=\"cdt.managedbuild.config.gnu.mingw.so.debug.366124197\">" << Endl;
				else
					os << L"<cconfiguration id=\"cdt.managedbuild.config.gnu.mingw.so.release.1986117871\">" << Endl;

				os << IncreaseIndent;

				os << DecreaseIndent;
				os << L"</cconfiguration>" << Endl;
			}

			os << DecreaseIndent;
			os << L"</storageModule>" << Endl;

			//os << L"<storageModule moduleId=\"cdtBuildSystem\" version=\"4.0.0\">" << Endl;
			//os << IncreaseIndent;
			//os << L"<project id=\"" << project->getName() << L".cdt.managedbuild.target.gnu.mingw.so.1811768793\" name=\"Shared Library\" projectType=\"cdt.managedbuild.target.gnu.mingw.so\"/>" << Endl;
			//os << DecreaseIndent;
			//os << L"</storageModule>" << Endl;

			os << DecreaseIndent;
			os << L"</cproject>" << Endl;

			os.close();

			if (!buffer.empty())
			{
				Ref< IStream > file = FileSystem::getInstance().open(
					cprojectFileName,
					traktor::File::FmWrite
				);
				if (!file)
					return false;
				file->write(&buffer[0], int(buffer.size()));
				file->close();
			}
			*/
		}
	}

	return true;
}

void SolutionBuilderEclipse::showOptions() const
{
}

void SolutionBuilderEclipse::addItem(const Solution* solution, const Project* project, const ProjectItem* item, const std::wstring& filterPath, OutputStream& os) const
{
	const Filter* filter = dynamic_type_cast< const Filter* >(item);
	if (filter)
	{
		os << L"<link>" << Endl;
		os << L"\t<name>" << filterPath << filter->getName() << L"</name>" << Endl;
		os << L"\t<type>2</type>" << Endl;
		os << L"\t<locationURI>virtual:/virtual</locationURI>" << Endl;
		os << L"</link>" << Endl;

		std::wstring childFilterPath = filterPath;

		if (childFilterPath.empty())
			childFilterPath = filter->getName() + L"/";
		else
			childFilterPath = childFilterPath + filter->getName() + L"/";

		const RefArray< ProjectItem >& items = item->getItems();
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
			addItem(solution, project, *i, childFilterPath, os);
	}

	const ::File* file = dynamic_type_cast< const ::File* >(item);
	if (file)
	{
		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();

		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);

		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		{
			Path systemFilePath = FileSystem::getInstance().getAbsolutePath(*i);

			Path relativeFilePath;
			FileSystem::getInstance().getRelativePath(
				systemFilePath,
				projectPath,
				relativeFilePath
			);

			os << L"<link>" << Endl;
			os << L"\t<name>" << filterPath << relativeFilePath.getFileName() << L"</name>" << Endl;
			os << L"\t<type>1</type>" << Endl;
			os << L"\t<locationURI>PROJECT_LOC/" << relativeFilePath.getPathName() << L"</locationURI>" << Endl;
			os << L"</link>" << Endl;
		}
	}
}
