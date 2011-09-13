#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/Settings.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

bool recursiveConvertInstances(db::Group* targetGroup, db::Group* sourceGroup)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	log::info << IncreaseIndent;

	RefArray< db::Instance > childInstances;
	sourceGroup->getChildInstances(childInstances);

	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		Ref< db::Instance > sourceInstance = *i;
		T_ASSERT (sourceInstance);

		log::info << L"Converting \"" << sourceInstance->getName() << L"\"..." << Endl;

		Ref< ISerializable > sourceObject = sourceInstance->getObject();
		if (!sourceObject)
		{
			log::error << L"Failed, unable to get source object" << Endl;
			return false;
		}

		Guid sourceGuid = sourceInstance->getGuid();
		Ref< db::Instance > targetInstance = targetGroup->createInstance(sourceInstance->getName(), db::CifReplaceExisting, &sourceGuid);
		if (!targetInstance)
		{
			log::error << L"Failed, unable to create target instance" << Endl;
			return false;
		}

		targetInstance->setObject(sourceObject);

		std::vector< std::wstring > dataNames;
		sourceInstance->getDataNames(dataNames);

		for (std::vector< std::wstring >::iterator j = dataNames.begin(); j != dataNames.end(); ++j)
		{
			log::info << L"\t\"" << *j << L"\"..." << Endl;

			Ref< IStream > sourceStream = sourceInstance->readData(*j);
			if (!sourceStream)
			{
				log::error << L"Failed, unable to open source stream" << Endl;
				return false;
			}

			Ref< IStream > targetStream = targetInstance->writeData(*j);
			if (!targetStream)
			{
				log::error << L"Failed, unable to open target stream" << Endl;
				return false;
			}

			if (!StreamCopy(targetStream, sourceStream).execute())
			{
				log::error << L"Failed, unable to copy data" << Endl;
				return false;
			}

			targetStream->close();
			sourceStream->close();
		}

		if (!targetInstance->commit())
		{
			log::error << L"Failed, unable to commit target instance" << Endl;
			return false;
		}
	}

	RefArray< db::Group > childGroups;
	sourceGroup->getChildGroups(childGroups);

	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< db::Group > sourceChildGroup = *i;
		T_ASSERT (sourceChildGroup);

		log::info << L"Creating group \"" << sourceChildGroup->getName() << L"\"..." << Endl;

		Ref< db::Group > targetChildGroup = targetGroup->getGroup(sourceChildGroup->getName());
		if (!targetChildGroup)
		{
			targetChildGroup = targetGroup->createGroup(sourceChildGroup->getName());
			if (!targetChildGroup)
			{
				log::error << L"Failed, unable to create target group" << Endl;
				return false;
			}
		}

		if (!recursiveConvertInstances(targetChildGroup, sourceChildGroup))
			return false;
	}

	return true;
}

Ref< Settings > loadSettings(const std::wstring& settingsFile)
{
	Ref< Settings > settings;
	Ref< traktor::IStream > file;

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	if (settings)
	{
		log::info << L"Using configuration \"" << userConfig << L"\"" << Endl;
		return settings;
	}

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		settings = Settings::read< xml::XmlDeserializer >(file);
		file->close();
	}

	if (settings)
	{
		log::info << L"Using configuration \"" << globalConfig << L"\"" << Endl;
		return settings;
	}

	return 0;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);

	log::info << L"Database Migration Tool; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	if (!cmdLine.hasOption('s', L"settings") && cmdLine.getCount() < 2)
	{
		log::info << L"Usage: Traktor.Database.Migrate.App [source database] [destination database] (module)*" << Endl;
		log::info << L"       Traktor.Database.Migrate.App -s|-settings=[settings]" << Endl;
		return 0;
	}

	std::wstring sourceCs;
	std::wstring destinationCs;

	if (cmdLine.hasOption('s', L"settings"))
	{
		std::wstring settingsFile = cmdLine.getOption('s', L"settings").getString();

		Ref< Settings > settings = loadSettings(settingsFile);
		if (!settings)
		{
			traktor::log::error << L"Unable to load migrate settings \"" << settingsFile << L"\"" << Endl;
			return 1;
		}

		std::vector< std::wstring > modules = settings->getProperty< PropertyStringArray >(L"Migrate.Modules");
		for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
		{
			Library library;
			if (!library.open(*i))
			{
				log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
				return 2;
			}
			library.detach();
		}

		sourceCs = settings->getProperty< PropertyString >(L"Migrate.SourceDatabase");
		destinationCs = settings->getProperty< PropertyString >(L"Migrate.OutputDatabase");
	}
	else
	{
		for (size_t i = 2; i < cmdLine.getCount(); ++i)
		{
			Library library;
			if (!library.open(cmdLine.getString(i)))
			{
				log::error << L"Unable to load module \"" << cmdLine.getString(i) << L"\"" << Endl;
				return 2;
			}
			library.detach();
		}

		sourceCs = cmdLine.getString(0);
		destinationCs = cmdLine.getString(1);
	}

	Ref< db::Database > sourceDb = new db::Database();
	if (!sourceDb->open(sourceCs))
	{
		log::error << L"Unable to open source database \"" << sourceCs << L"\"" << Endl;
		return 3;
	}

	Ref< db::Database > destinationDb = new db::Database();
	if (!destinationDb->create(destinationCs))
	{
		log::error << L"Unable to create destination database \"" << destinationCs << L"\"" << Endl;
		return 4;
	}

	log::info << L"Migration begin" << Endl;

	Ref< db::Group > sourceGroup = sourceDb->getRootGroup();
	Ref< db::Group > targetGroup = destinationDb->getRootGroup();
	if (sourceGroup && targetGroup)
	{
		if (!recursiveConvertInstances(targetGroup, sourceGroup))
			return 5;
	}

	log::info << L"Migration complete" << Endl;

	destinationDb->close();
	sourceDb->close();

	return 0;
}
