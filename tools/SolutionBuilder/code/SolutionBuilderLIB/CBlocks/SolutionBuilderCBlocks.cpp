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
#include "SolutionBuilderLIB/Solution.h"
#include "SolutionBuilderLIB/CBlocks/SolutionBuilderCBlocks.h"

using namespace traktor;

T_IMPLEMENT_RTTI_CLASS(L"SolutionBuilderCBlocks", SolutionBuilderCBlocks, SolutionBuilder)

SolutionBuilderCBlocks::SolutionBuilderCBlocks()
:	m_compiler(L"msvc8")
{
}

bool SolutionBuilderCBlocks::create(const traktor::CommandLine& cmdLine)
{
	if (cmdLine.hasOption('c'))
		m_compiler = cmdLine.getOption('c').getString();
	return true;
}

bool SolutionBuilderCBlocks::generate(Solution* solution)
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
		std::wstring projectFileName = projectPath + L"/" + project->getName() + L".cbp";

		if (!FileSystem::getInstance().makeDirectory(projectPath))
			return false;

		std::vector< uint8_t > buffer;
		buffer.reserve(40000);

		DynamicMemoryStream bufferStream(buffer, false, true);
		FileOutputStream os(&bufferStream, new Utf8Encoding());

		os << L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>" << Endl;
		os << L"<CodeBlocks_project_file>" << Endl;
		os << IncreaseIndent;

		os << L"<FileVersion major=\"1\" minor=\"6\" />" << Endl;
		os << L"<Project>" << Endl;
		os << IncreaseIndent;

		os << L"<Option title=\"" << project->getName() << L"\" />" << Endl;
		os << L"<Option compiler=\"" << m_compiler << L"\" />" << Endl;

		os << L"<Build>" << Endl;
		os << IncreaseIndent;

		const RefArray< Configuration >& configurations = project->getConfigurations();
		for (RefArray< Configuration >::const_iterator j = configurations.begin(); j != configurations.end(); ++j)
		{
			const Configuration* configuration = *j;

			os << L"<Target title=\"" << configuration->getName() << L"\">" << Endl;
			os << IncreaseIndent;

			std::wstring outputPath = L"..\\" + configuration->getName();
			std::wstring intermediatePath = L"obj\\" + configuration->getName();

			if (m_compiler == L"msvc8")
			{
				switch (configuration->getTargetFormat())
				{
				case Configuration::TfStaticLibrary:
					os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L".lib\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				case Configuration::TfSharedLibrary:
					os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L".dll\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				case Configuration::TfExecutable:
					os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L".exe\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				case Configuration::TfExecutableConsole:
					os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L".exe\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				}
			}
			else if (m_compiler == L"gcc")
			{
				switch (configuration->getTargetFormat())
				{
				case Configuration::TfStaticLibrary:
					os << L"<Option output=\"" << outputPath << L"\\lib" << project->getName() << L".a\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				case Configuration::TfSharedLibrary:
					os << L"<Option output=\"" << outputPath << L"\\lib" << project->getName() << L".so\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				case Configuration::TfExecutable:
					os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L"\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				case Configuration::TfExecutableConsole:
					os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L"\" prefix_auto=\"0\" extension_auto=\"0\" />" << Endl;
					break;
				}
			}
			else
				os << L"<Option output=\"" << outputPath << L"\\" << project->getName() << L"\" prefix_auto=\"1\" extension_auto=\"1\" />" << Endl;

			os << L"<Option object_output=\"" << intermediatePath << L"\" />" << Endl;
			os << L"<Option compiler=\"" << m_compiler << L"\" />" << Endl;

			switch (configuration->getTargetFormat())
			{
			case Configuration::TfStaticLibrary:
				os << L"<Option type=\"2\" />" << Endl;
				break;
			case Configuration::TfSharedLibrary:
				os << L"<Option type=\"3\" />" << Endl;
				os << L"<Option createDefFile=\"0\" />" << Endl;
				os << L"<Option createStaticLib=\"1\" />" << Endl;
				break;
			case Configuration::TfExecutable:
				os << L"<Option type=\"0\" />" << Endl;
				break;
			case Configuration::TfExecutableConsole:
				os << L"<Option type=\"1\" />" << Endl;
				break;
			}
			
			os << L"<Compiler>" << Endl;
			os << IncreaseIndent;

			if (m_compiler == L"msvc8")
			{
				if (configuration->getTargetProfile() == Configuration::TpDebug)
				{
					os << L"<Add option=\"/Od\" />" << Endl;
					os << L"<Add option=\"/Zi\" />" << Endl;
					os << L"<Add option=\"/MDd\" />" << Endl;
				}
				else
				{
					os << L"<Add option=\"/Ox\" />" << Endl;
					os << L"<Add option=\"/Ob2\" />" << Endl;
					os << L"<Add option=\"/Oi\" />" << Endl;
					os << L"<Add option=\"/Ot\" />" << Endl;
					os << L"<Add option=\"/Oy\" />" << Endl;
					os << L"<Add option=\"/fp:fast\" />" << Endl;
					os << L"<Add option=\"/MD\" />" << Endl;
				}

				os << L"<Add option=\"/EHsc\" />" << Endl;
				os << L"<Add option=\"/arch:SSE2\" />" << Endl;

				switch (configuration->getTargetFormat())
				{
				case Configuration::TfStaticLibrary:
					os << L"<Add option=\"/D _LIBRARY\" />" << Endl;
					os << L"<Add option=\"/D _WINDOWS\" />" << Endl;
					break;
				case Configuration::TfSharedLibrary:
					os << L"<Add option=\"/D _USRDLL\" />" << Endl;
					os << L"<Add option=\"/D _WINDLL\" />" << Endl;
					os << L"<Add option=\"/D _WINDOWS\" />" << Endl;
					break;
				case Configuration::TfExecutable:
					os << L"<Add option=\"/D _WINDOWS\" />" << Endl;
					break;
				case Configuration::TfExecutableConsole:
					os << L"<Add option=\"/D _CONSOLE\" />" << Endl;
					break;
				}

				os << L"<Add option=\"/D WIN32\" />" << Endl;
				os << L"<Add option=\"/D UNICODE\" />" << Endl;
				os << L"<Add option=\"/D _UNICODE\" />" << Endl;
			}
			else if (m_compiler == L"gcc")
			{
				if (configuration->getTargetProfile() == Configuration::TpDebug)
					os << L"<Add option=\"-g\" />" << Endl;
				else
					os << L"<Add option=\"-O2\" />" << Endl;

				if (configuration->getTargetFormat() == Configuration::TfSharedLibrary)
					os << L"<Add option=\"-fPIC\" />" << Endl;

				os << L"<Add option=\"-DUNICODE\" />" << Endl;
				os << L"<Add option=\"-D_UNICODE\" />" << Endl;
			}

			const std::vector< std::wstring >& definitions = configuration->getDefinitions();
			for (std::vector< std::wstring >::const_iterator k = definitions.begin(); k != definitions.end(); ++k)
				os << L"<Add option=\"-D" << *k << L"\" />" << Endl;

			const std::vector< std::wstring >& includePaths = configuration->getIncludePaths();
			for (std::vector< std::wstring >::const_iterator k = includePaths.begin(); k != includePaths.end(); ++k)
			{
				Path relativePath;
				FileSystem::getInstance().getRelativePath(
					*k,
					projectPath,
					relativePath
				);
				os << L"<Add directory=\"" << relativePath.getPathName() << L"\" />" << Endl;
			}

			os << L"<Add directory=\".\" />" << Endl;

			os << DecreaseIndent;
			os << L"</Compiler>" << Endl;

			if (
				configuration->getTargetFormat() == Configuration::TfSharedLibrary ||
				configuration->getTargetFormat() == Configuration::TfExecutable ||
				configuration->getTargetFormat() == Configuration::TfExecutableConsole
			)
			{
				os << L"<Linker>" << Endl;				os << IncreaseIndent;

				if (m_compiler == L"msvc8")
				{
					if (configuration->getTargetProfile() == Configuration::TpDebug)
						os << L"<Add option=\"/DEBUG\" />" << Endl;					os << L"<Add library=\"kernel32\" />" << Endl;
					os << L"<Add library=\"user32\" />" << Endl;
					os << L"<Add library=\"gdi32\" />" << Endl;
					os << L"<Add library=\"winspool\" />" << Endl;
					os << L"<Add library=\"comdlg32\" />" << Endl;
					os << L"<Add library=\"advapi32\" />" << Endl;
					os << L"<Add library=\"shell32\" />" << Endl;
					os << L"<Add library=\"ole32\" />" << Endl;
					os << L"<Add library=\"oleaut32\" />" << Endl;
					os << L"<Add library=\"uuid\" />" << Endl;
					os << L"<Add library=\"odbc32\" />" << Endl;
					os << L"<Add library=\"odbccp32\" />" << Endl;
				}

				const std::vector< std::wstring >& libraryPaths = configuration->getLibraryPaths();
				for (std::vector< std::wstring >::const_iterator k = libraryPaths.begin(); k != libraryPaths.end(); ++k)
				{
					Path relativePath;
					FileSystem::getInstance().getRelativePath(
						*k,
						projectPath,
						relativePath
					);
					os << L"<Add directory=\"" << relativePath.getPathName() << L"\" />" << Endl;
				}

				const std::vector< std::wstring >& libraries = configuration->getLibraries();
				for (std::vector< std::wstring >::const_iterator k = libraries.begin(); k != libraries.end(); ++k)
					os << L"<Add library=\"" << *k << L"\" />" << Endl;

				// Link to dependency products.
				RefArray< Dependency > dependencies = project->getDependencies();
				if (!dependencies.empty())
				{
					std::set< Path > dependencyPaths;
					std::set< std::wstring > dependencyProducts;

					while (!dependencies.empty())
					{
						const Dependency* dependency = dependencies.front(); dependencies.pop_front();

						const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(dependency);
						if (projectDependency)
						{
							const Project* projectDep = projectDependency->getProject();
							const Configuration* configurationDep = projectDep->getConfiguration(configuration->getName());
							if (configurationDep)
							{
								dependencyPaths.insert(outputPath);
								dependencyProducts.insert(projectDep->getName());

								// Recursively add dependencies from static libraries.
								if (configurationDep->getTargetFormat() == Configuration::TfStaticLibrary)
								{
									for (RefArray< Dependency >::const_iterator k = projectDep->getDependencies().begin(); k != projectDep->getDependencies().end(); ++k)
										dependencies.push_back(*k);

									//const std::vector< std::wstring >& libraryPathsDep = configurationDep->getLibraryPaths();
									//for (const std::vector< std::wstring >::const_iterator k = libraryPathsDep.begin(); k != libraryPathsDep.end(); ++k)
									//{
									//	Path relativePath;
									//	FileSystem::getInstance().getRelativePath(
									//		*k,
									//		projectPath,
									//		relativePath
									//	);
									//	dependencyPaths.insert(relativePath);
									//}

									const std::vector< std::wstring >& librariesDep = configurationDep->getLibraries();
									for (std::vector< std::wstring >::const_iterator k = librariesDep.begin(); k != librariesDep.end(); ++k)
										dependencyProducts.insert(*k);
								}
							}
						}

						const ExternalDependency* externalDependency = dynamic_type_cast< const ExternalDependency* >(dependency);
						if (externalDependency)
						{
							const Solution* solutionDep = externalDependency->getSolution();
							const Project* projectDep = externalDependency->getProject();
							const Configuration* configurationDep = projectDep->getConfiguration(configuration->getName());
							if (configurationDep)
							{
								Path outputDepRelativePath;
								FileSystem::getInstance().getRelativePath(
									solutionDep->getRootPath() + L"/" + configuration->getName(),
									projectPath,
									outputDepRelativePath
								);

								dependencyPaths.insert(outputDepRelativePath);
								dependencyProducts.insert(projectDep->getName());

								// Recursively add dependencies from static libraries.
								if (configurationDep->getTargetFormat() == Configuration::TfStaticLibrary)
								{
									for (RefArray< Dependency >::const_iterator k = projectDep->getDependencies().begin(); k != projectDep->getDependencies().end(); ++k)
										dependencies.push_back(*k);

									//const std::vector< std::wstring >& libraryPathsDep = configurationDep->getLibraryPaths();
									//for (const std::vector< std::wstring >::const_iterator k = libraryPathsDep.begin(); k != libraryPathsDep.end(); ++k)
									//{
									//	Path relativePath;
									//	FileSystem::getInstance().getRelativePath(
									//		*k,
									//		projectPath,
									//		relativePath
									//	);
									//	dependencyPaths.insert(relativePath);
									//}

									const std::vector< std::wstring >& librariesDep = configurationDep->getLibraries();
									for (std::vector< std::wstring >::const_iterator k = librariesDep.begin(); k != librariesDep.end(); ++k)
										dependencyProducts.insert(*k);
								}
							}
						}
					}

					for (std::set< Path >::const_iterator k = dependencyPaths.begin(); k != dependencyPaths.end(); ++k)
						os << L"<Add directory=\"" << k->getPathName() << L"\" />" << Endl;

					for (std::set< std::wstring >::const_iterator k = dependencyProducts.begin(); k != dependencyProducts.end(); ++k)
						os << L"<Add library=\"" << *k << L"\" />" << Endl;
				}

				os << DecreaseIndent;				os << L"</Linker>" << Endl;			}

			os << DecreaseIndent;
			os << L"</Target>" << Endl;
		}

		os << DecreaseIndent;
		os << L"</Build>" << Endl;

		// Add extra step to prepare embed items.
		std::set< Path > embedItems;
		for (RefArray< ProjectItem >::const_iterator k = items.begin(); k != items.end(); ++k)
			collectEmbedItems(solution, project, *k, embedItems);

		if (!embedItems.empty())
		{
			os << L"<ExtraCommands>" << Endl;
			os << IncreaseIndent;

			for (std::set< Path >::const_iterator k = embedItems.begin(); k != embedItems.end(); ++k)
			{
				Path binaryIncludePath(L"$(TRAKTOR_HOME)/bin/BinaryInclude");
				os << L"<Add before=\"" << binaryIncludePath.getPathName() << L" " << k->getPathName() << L" ./Resources/" << k->getFileNameNoExtension() << L".h c_Resource" << k->getFileNameNoExtension() << L"\" />" << Endl;
			}

			os << DecreaseIndent;
			os << L"</ExtraCommands>" << Endl;
		}

		os << L"<Compiler>" << Endl;
		os << IncreaseIndent;
		if (m_compiler == L"msvc8")
			os << L"<Add option=\"/W3\" />" << Endl;
		else
			os << L"<Add option=\"-Wall\" />" << Endl;
		os << DecreaseIndent;
		os << L"</Compiler>" << Endl;

		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
			addItem(solution, project, *i, os);

		os << L"<Extensions>" << Endl;
		os << IncreaseIndent;
		os << L"<code_completion />" << Endl;
		os << L"<debugger />" << Endl;
		os << DecreaseIndent;
		os << L"</Extensions>" << Endl;

		os << DecreaseIndent;
		os << L"</Project>" << Endl;

		os << DecreaseIndent;
		os << L"</CodeBlocks_project_file>" << Endl;

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
	}

	// Generate workspace.
	{
		std::wstring workspacePath = solution->getRootPath();
		std::wstring workspaceFileName = workspacePath + L"/" + solution->getName() + L".workspace";

		std::vector< uint8_t > buffer;
		buffer.reserve(40000);

		DynamicMemoryStream bufferStream(buffer, false, true);
		FileOutputStream os(&bufferStream, new Utf8Encoding());

		os << L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\" ?>" << Endl;
		os << L"<CodeBlocks_workspace_file>" << Endl;
		os << IncreaseIndent;
		os << L"<Workspace title=\"" << solution->getName() << L"\">" << Endl;
		os << IncreaseIndent;

		for (RefArray< Project >::const_iterator i = projects.begin(); i != projects.end(); ++i)
		{
			const Project* project = *i;

			// Skip disabled projects.
			if (!project->getEnable())
				continue;

			std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();
			std::wstring projectFileName = projectPath + L"/" + project->getName() + L".cbp";

			Path projectRelativePath;
			FileSystem::getInstance().getRelativePath(
				projectFileName,
				workspacePath,
				projectRelativePath
			);

			const RefArray< Dependency >& dependencies = project->getDependencies();
			if (!dependencies.empty())
			{
				os << L"<Project filename=\"" << projectRelativePath.getPathName() << L"\">" << Endl;
				os << IncreaseIndent;

				for (RefArray< Dependency >::const_iterator j = dependencies.begin(); j != dependencies.end(); ++j)
				{
					const ProjectDependency* projectDependency = dynamic_type_cast< const ProjectDependency* >(*j);
					if (projectDependency)
					{
						const Project* projectDep = projectDependency->getProject();
						T_ASSERT (projectDep);

						std::wstring projectDepFileName = projectDep->getName() + L"/" + projectDep->getName() + L".cbp";

						os << L"<Depends filename=\"" << projectDepFileName << L"\" />" << Endl;
					}
				}

				os << DecreaseIndent;
				os << L"</Project>" << Endl;
			}
			else
				os << L"<Project filename=\"" << projectRelativePath.getPathName() << L"\" />" << Endl;
		}

		os << DecreaseIndent;
		os << L"</Workspace>" << Endl;
		os << DecreaseIndent;
		os << L"</CodeBlocks_workspace_file>" << Endl;

		os.close();

		if (!buffer.empty())
		{
			Ref< IStream > file = FileSystem::getInstance().open(
				workspaceFileName,
				traktor::File::FmWrite
			);
			if (!file)
				return false;
			file->write(&buffer[0], int(buffer.size()));
			file->close();
		}
	}

	return true;
}

void SolutionBuilderCBlocks::showOptions() const
{
	traktor::log::info << L"\t-c=[compiler]	(\"msvc8\", \"gcc\")" << Endl;
}

void SolutionBuilderCBlocks::collectEmbedItems(const Solution* solution, const Project* project, const ProjectItem* item, std::set< Path >& outEmbedItems) const
{
	const Filter* filter = dynamic_type_cast< const Filter* >(item);
	if (filter)
	{
		const RefArray< ProjectItem >& items = item->getItems();
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
			collectEmbedItems(solution, project, *i, outEmbedItems);
	}

	const ::File* file = dynamic_type_cast< const ::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);

		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		{
			std::wstring extension = toLower(i->getExtension());
			if (extension == L"png" || extension == L"xdi")
			{
				Path relativeFilePath;
				FileSystem::getInstance().getRelativePath(
					*i,
					solution->getRootPath() + L"/" + project->getName(),
					relativeFilePath
				);
				outEmbedItems.insert(relativeFilePath);
			}
		}
	}
}

void SolutionBuilderCBlocks::addItem(const Solution* solution, const Project* project, const ProjectItem* item, OutputStream& os) const
{
	const Filter* filter = dynamic_type_cast< const Filter* >(item);
	if (filter)
	{
		const RefArray< ProjectItem >& items = item->getItems();
		for (RefArray< ProjectItem >::const_iterator i = items.begin(); i != items.end(); ++i)
			addItem(solution, project, *i, os);
	}

	const ::File* file = dynamic_type_cast< const ::File* >(item);
	if (file)
	{
		std::set< Path > systemFiles;
		file->getSystemFiles(project->getSourcePath(), systemFiles);

		std::wstring projectPath = solution->getRootPath() + L"/" + project->getName();

		for (std::set< Path >::iterator i = systemFiles.begin(); i != systemFiles.end(); ++i)
		{
			Path systemFilePath = FileSystem::getInstance().getAbsolutePath(*i);

			Path relativeFilePath;
			FileSystem::getInstance().getRelativePath(
				systemFilePath,
				projectPath,
				relativeFilePath
			);

			std::wstring extension = toLower(relativeFilePath.getExtension());
			if (extension == L"c" || extension == L"cc" || extension == L"cpp")
			{
				os << L"<Unit filename=\"" << relativeFilePath.getPathName() << L"\" />" << Endl;
			}
			else if (extension == L"h" || extension == L"hpp")
			{
				os << L"<Unit filename=\"" << relativeFilePath.getPathName() << L"\" />" << Endl;
			}
		}
	}
}