/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Deploy/BuildTargetAction.h"
#include "Runtime/Editor/Deploy/DeployTargetAction.h"
#include "Runtime/Editor/Deploy/LaunchTargetAction.h"
#include "Runtime/Editor/Deploy/MigrateTargetAction.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Database/Local/LocalDatabase.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

std::wstring mangleName(std::wstring name)
{
	name = replaceAll(name, ' ', '_');
	name = replaceAll(name, ',', '_');
	name = replaceAll(name, ';', '_');
	name = replaceAll(name, '|', '_');
	return name;
}

Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< PropertyGroup > settings;
	Ref< traktor::IStream > file;

	if ((file = FileSystem::getInstance().open(settingsFile, File::FmRead)) != 0)
	{
		if ((settings = xml::XmlDeserializer(file).readObject< PropertyGroup >()) == 0)
			log::error << L"File " << settingsFile << L" is corrupt; unable to parse XML." << Endl;
		file->close();
	}

	return settings;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	if (cmdLine.getCount() < 4)
	{
		log::info << L"Traktor.Runtime.Deploy.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << Endl;
		log::info << L"Usage: Traktor.Runtime.Deploy.App (option(s)) [workspace] [command] [target] [configuration] (deploy target ip/hostname)" << Endl;
		log::info << Endl;
		log::info << L"  Options:" << Endl;
		log::info << L"    -s,-settings               Settings file (default \"$(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Editor.config\")" << Endl;
		log::info << L"    -v,-verbose                Verbose building." << Endl;
		log::info << L"    -f,-force                  Force build." << Endl;
		log::info << L"    -debug                     Use debug binaries in deploy or migrate actions." << Endl;
		log::info << L"    -static-link               Statically link product in deploy or migrate actions." << Endl;
		log::info << L"    -avalanche-cache=host:port Specify pipeline avalanche host." << Endl;
		log::info << L"    -file-cache                Specify pipeline file cache directory." << Endl;
		log::info << L"    -sequential-depends        Disable multithreaded pipeline dependency scanner." << Endl;
		return 1;
	}

	// Check if environment is already set, else set to current working directory.
	std::wstring home;
	if (!OS::getInstance().getEnvironment(L"TRAKTOR_HOME", home))
	{
		const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();

		const Path executablePath = OS::getInstance().getExecutable().getPathOnly();
		FileSystem::getInstance().setCurrentVolumeAndDirectory(executablePath);

		while (!FileSystem::getInstance().exist(L"LICENSE.txt"))
		{
			const Path cwd = FileSystem::getInstance().getCurrentVolumeAndDirectory();
			const Path pwd = cwd.getPathOnly();
			if (cwd == pwd)
			{
				log::error << L"No LICENSE.txt file found." << Endl;
				return 1;
			}
			FileSystem::getInstance().setCurrentVolumeAndDirectory(pwd);
		}

		OS::getInstance().setEnvironment(L"TRAKTOR_HOME", FileSystem::getInstance().getCurrentVolumeAndDirectory().getPathNameOS());
		FileSystem::getInstance().setCurrentVolumeAndDirectory(cwd);
	}

	std::wstring settingsFile = L"$(TRAKTOR_HOME)/resources/runtime/configurations/Traktor.Editor.config";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	std::wstring workspaceFile = cmdLine.getString(0);
	std::wstring command = cmdLine.getString(1);
	std::wstring targetPath = cmdLine.getString(2);
	std::wstring configuration = cmdLine.getString(3);

	Ref< PropertyGroup > settings;
	if ((settings = loadSettings(settingsFile)) == 0)
	{
		traktor::log::error << L"Unable to load settings \"" << settingsFile << L"\"." << Endl;
		return 1;
	}

	Ref< PropertyGroup > workspace;
	if ((workspace = loadSettings(workspaceFile)) == 0)
	{
		traktor::log::error << L"Unable to load workspace \"" << workspaceFile << L"\"." << Endl;
		return 1;
	}

	if (workspace)
		settings = settings->merge(workspace, PropertyGroup::MmReplace);

	if (cmdLine.hasOption(L"debug"))
		settings->setProperty< PropertyBoolean >(L"Runtime.UseDebugBinaries", true);
	if (cmdLine.hasOption(L"static-link"))
		settings->setProperty< PropertyBoolean >(L"Runtime.StaticallyLinked", true);

	// If cache has is explicitly set then we first clear property to ensure exclusivly enabled.
	if (cmdLine.hasOption(L"avalanche-cache") || cmdLine.hasOption(L"file-cache"))
	{
		settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache", false);
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", false);
	}

	if (cmdLine.hasOption(L"avalanche-cache"))
	{
		std::wstring host = cmdLine.getOption(L"avalanche-cache").getString();
		int32_t port = 40001;

		size_t i = host.find(L':');
		if (i != std::wstring::npos)
		{
			port = parseString< int32_t >(host.substr(i + 1));
			host = host.substr(0, i);
		}

		settings->setProperty< PropertyBoolean >(L"Runtime.InheritCache", true);
		settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache", true);
		settings->setProperty< PropertyString >(L"Pipeline.AvalancheCache.Host", host);
		settings->setProperty< PropertyInteger >(L"Pipeline.AvalancheCache.Port", port);
		settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache.Read", true);
		settings->setProperty< PropertyBoolean >(L"Pipeline.AvalancheCache.Write", true);
	}

	if (cmdLine.hasOption(L"file-cache"))
	{
		settings->setProperty< PropertyBoolean >(L"Runtime.InheritCache", true);
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache", true);
		settings->setProperty< PropertyString >(L"Pipeline.FileCache.Path", cmdLine.getOption(L"file-cache").getString());
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", true);
		settings->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", true);
	}

	if (cmdLine.hasOption(L"sequential-depends"))
		settings->setProperty< PropertyBoolean >(L"Pipeline.DependsThreads", false);
	if (cmdLine.hasOption(L'v', L"verbose"))
		settings->setProperty< PropertyBoolean >(L"Pipeline.Verbose", true);

	db::ConnectionString sourceDatabaseCS = settings->getProperty< std::wstring >(L"Editor.SourceDatabase");
	sourceDatabaseCS.set(L"fileStore", L"");

	T_FORCE_LINK_REF(db::LocalDatabase);

	Ref< db::Database > sourceDatabase = new db::Database();
	if (!sourceDatabase->open(sourceDatabaseCS))
	{
		traktor::log::error << L"Unable to open database \"" << sourceDatabaseCS.format() << L"\"." << Endl;
		return 1;
	}

	Ref< db::Instance > targetInstance = sourceDatabase->getInstance(targetPath);
	if (!targetInstance)
	{
		traktor::log::error << L"No such target \"" << targetPath << L"\"." << Endl;
		return 1;
	}

	Ref< const runtime::Target > target = targetInstance->getObject< runtime::Target >();
	if (!target)
	{
		traktor::log::error << L"Unable to read target \"" << targetPath << L"\"." << Endl;
		return 1;
	}

	Ref< runtime::TargetConfiguration > targetConfiguration;
	for (auto tc : target->getConfigurations())
	{
		if (tc->getName() == configuration)
		{
			targetConfiguration = tc;
			break;
		}
	}
	if (!targetConfiguration)
	{
		traktor::log::error << L"No such target configuration \"" << configuration << L"\", available configurations are:" << Endl;
		for (auto tc : target->getConfigurations())
			traktor::log::error << L"\t\"" << tc->getName() << L"\"" << Endl;
		return 1;
	}

	std::wstring outputPath = L"output/" + mangleName(targetInstance->getName()) + L"/" + mangleName(targetConfiguration->getName());
	std::wstring databaseName = mangleName(targetInstance->getName()) + L"|" + mangleName(targetConfiguration->getName());
	uint16_t databasePort = 0;
	std::wstring editorHost = L"";
	std::wstring deployHost = L"";
	uint16_t targetManagerPort = 0;
	Guid targetManagerId;

	if (cmdLine.getCount() >= 5)
		deployHost = cmdLine.getString(4);

	bool result = false;
	if (command == L"build")
	{
		result = runtime::BuildTargetAction(
			sourceDatabase,
			settings,
			nullptr,
			target,
			targetConfiguration,
			outputPath,
			nullptr,
			cmdLine.hasOption(L'f', L"force")
		).execute(nullptr);
	}
	else if (command == L"deploy")
	{
		result = runtime::DeployTargetAction(
			sourceDatabase,
			settings,
			targetInstance->getName(),
			target,
			targetConfiguration,
			editorHost,
			deployHost,
			databasePort,
			databaseName,
			targetManagerPort,
			targetManagerId,
			outputPath,
			nullptr
		).execute(nullptr);
	}
	else if (command == L"launch")
	{
		result = runtime::LaunchTargetAction(
			sourceDatabase,
			settings,
			targetInstance->getName(),
			target,
			targetConfiguration,
			deployHost,
			outputPath
		).execute(nullptr);
	}
	else if (command == L"migrate")
	{
		result = runtime::MigrateTargetAction(
			sourceDatabase,
			settings,
			targetInstance->getName(),
			target,
			targetConfiguration,
			deployHost,
			outputPath
		).execute(nullptr);
	}
	else
	{
		traktor::log::error << L"Unknown command \"" << cmdLine.getString(0) << L"\"." << Endl;
		return 1;
	}

	return result ? 0 : 1;
}
