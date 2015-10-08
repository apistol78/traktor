#include "Amalgam/Editor/Deploy/BuildTargetAction.h"
#include "Amalgam/Editor/Deploy/DeployTargetAction.h"
#include "Amalgam/Editor/Deploy/LaunchTargetAction.h"
#include "Amalgam/Editor/Deploy/MigrateTargetAction.h"
#include "Amalgam/Editor/Deploy/Target.h"
#include "Amalgam/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
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
		log::info << L"Traktor.Amalgam.Deploy.App; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		log::info << Endl;
		log::info << L"Usage: Traktor.Amalgam.Deploy.App (option(s)) [workspace] [command] [target] [configuration]" << Endl;
		log::info << Endl;
		log::info << L"  Options:" << Endl;
		log::info << L"    -s,-settings              Settings file (default \"$(TRAKTOR_HOME)/Traktor.Editor.config\")" << Endl;
		log::info << L"    -standalone               Build using a standalone pipeline." << Endl;
		log::info << L"    -debug                    Use debug binaries in deploy or migrate actions." << Endl;
		log::info << L"    -static-link              Statically link product in deploy or migrate actions." << Endl;
		return 1;
	}

	std::wstring settingsFile = L"$(TRAKTOR_HOME)/Traktor.Editor.config";
	if (cmdLine.hasOption('s', L"settings"))
		settingsFile = cmdLine.getOption('s', L"settings").getString();

	std::wstring workspaceFile = cmdLine.getString(0);
	std::wstring command = cmdLine.getString(1);
	std::wstring targetPath = cmdLine.getString(2);
	std::wstring configuration = cmdLine.getString(3);

	Ref< PropertyGroup > settings;
	if ((settings = loadSettings(settingsFile)) == 0)
	{
		traktor::log::error << L"Unable to load settings \"" << settingsFile << L"\"" << Endl;
		return 1;
	}

	Ref< PropertyGroup > workspace;
	if ((workspace = loadSettings(workspaceFile)) == 0)
	{
		traktor::log::error << L"Unable to load workspace \"" << workspaceFile << L"\"" << Endl;
		return 1;
	}

	if (workspace)
		settings = settings->mergeReplace(workspace);

	if (cmdLine.hasOption(L"debug"))
		settings->setProperty< PropertyBoolean >(L"Amalgam.UseDebugBinaries", true);
	if (cmdLine.hasOption(L"static-link"))
		settings->setProperty< PropertyBoolean >(L"Amalgam.StaticallyLinked", true);

	db::ConnectionString sourceDatabaseCS = settings->getProperty< PropertyString >(L"Editor.SourceDatabase");
	sourceDatabaseCS.set(L"fileStore", L"");

	T_FORCE_LINK_REF(db::LocalDatabase);

	Ref< db::Database > sourceDatabase = new db::Database();
	if (!sourceDatabase->open(sourceDatabaseCS))
	{
		traktor::log::error << L"Unable to open database \"" << sourceDatabaseCS.format() << L"\"" << Endl;
		return 1;
	}

	Ref< db::Instance > targetInstance = sourceDatabase->getInstance(targetPath);
	if (!targetInstance)
	{
		traktor::log::error << L"No such target \"" << targetPath << L"\"" << Endl;
		return 1;
	}

	Ref< const amalgam::Target > target = targetInstance->getObject< amalgam::Target >();
	if (!target)
	{
		traktor::log::error << L"No such target \"" << targetPath << L"\"" << Endl;
		return 1;
	}

	Ref< amalgam::TargetConfiguration > targetConfiguration;
	const RefArray< amalgam::TargetConfiguration >& targetConfigurations = target->getConfigurations();
	for (RefArray< amalgam::TargetConfiguration >::const_iterator i = targetConfigurations.begin(); i != targetConfigurations.end(); ++i)
	{
		if ((*i)->getName() == configuration)
		{
			targetConfiguration = *i;
			break;
		}
	}
	if (!targetConfiguration)
	{
		traktor::log::error << L"No such target configuration \"" << configuration << L"\"" << Endl;
		return 1;
	}

	std::wstring outputPath = L"output/" + mangleName(targetInstance->getName()) + L"/" + mangleName(targetConfiguration->getName());
	std::wstring databaseName = mangleName(targetInstance->getName()) + L"|" + mangleName(targetConfiguration->getName());
	uint16_t databasePort = 0;
	std::wstring editorHost = L"";
	std::wstring deployHost = L"";
	uint16_t targetManagerPort = 0;
	Guid targetManagerId;

	bool result = false;
	if (command == L"build")
	{
		result = amalgam::BuildTargetAction(
			sourceDatabase,
			settings,
			0,
			target,
			targetConfiguration,
			outputPath,
			cmdLine.hasOption(L"standalone")
		).execute(0);
	}
	else if (command == L"deploy")
	{
		result = amalgam::DeployTargetAction(
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
			0
		).execute(0);
	}
	else if (command == L"launch")
	{
		result = amalgam::LaunchTargetAction(
			sourceDatabase,
			settings,
			targetInstance->getName(),
			target,
			targetConfiguration,
			deployHost,
			outputPath
		).execute(0);
	}
	else if (command == L"migrate")
	{
		result = amalgam::MigrateTargetAction(
			sourceDatabase,
			settings,
			targetInstance->getName(),
			target,
			targetConfiguration,
			deployHost,
			outputPath
		).execute(0);
	}
	else
	{
		traktor::log::error << L"Unknown command \"" << cmdLine.getString(0) << L"\"" << Endl;
		return 1;
	}

	return result ? 0 : 1;
}
