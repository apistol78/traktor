/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Log/LogRedirectTarget.h"
#include "Core/Log/LogStreamTarget.h"
#include "Core/Misc/CommandLine.h"
#include "Core/Misc/TString.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/OS.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Xml/XmlDeserializer.h"

using namespace traktor;

namespace
{

/*! Count number of groups in database.
 *
 * \param group Current group
 * \reurn Number of groups including current group.
 */
int32_t countGroups(db::Group* group)
{
	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	int32_t localCount = 1;
	for (auto childGroup : childGroups)
		localCount += countGroups(childGroup);

	return localCount;
}

/*! Migrate instance from source into target group.
 *
 * \param sourceInstance Source instance to migrate.
 * \param targetGroup Migrate into target group.
 */
bool migrateInstance(Ref< db::Instance > sourceInstance, Ref< db::Group > targetGroup, const DateTime& modifiedSince)
{
	// First check if source instance has been modified since last migration, if not
	// then we can quickly ignore migration of instance.
	DateTime sourceModifyDate;
	sourceInstance->getLastModifyDate(sourceModifyDate);
	if (sourceModifyDate < modifiedSince)
		return true;

	Ref< ISerializable > sourceObject = sourceInstance->getObject();
	if (!sourceObject)
	{
		traktor::log::error << L"Failed, unable to get source object." << Endl;
		return false;
	}

	Guid sourceGuid = sourceInstance->getGuid();

	std::vector< std::wstring > dataNames;
	sourceInstance->getDataNames(dataNames);

	Ref< db::Instance > targetInstance = targetGroup->createInstance(sourceInstance->getName(), db::CifReplaceExisting, &sourceGuid);
	if (!targetInstance)
	{
		traktor::log::error << L"Failed, unable to create target instance." << Endl;
		return false;
	}

	targetInstance->setObject(sourceObject);

	for (const auto& dataName : dataNames)
	{
		Ref< IStream > sourceStream = sourceInstance->readData(dataName);
		if (!sourceStream)
		{
			traktor::log::error << L"Failed, unable to open source stream \"" << dataName << L"." << Endl;
			return false;
		}

		Ref< IStream > targetStream = targetInstance->writeData(dataName);
		if (!targetStream)
		{
			traktor::log::error << L"Failed, unable to open target stream \"" << dataName << L"." << Endl;
			return false;
		}

		if (!StreamCopy(targetStream, sourceStream).execute())
		{
			traktor::log::error << L"Failed, unable to copy data \"" << dataName << L"." << Endl;
			return false;
		}

		targetStream->close();
		sourceStream->close();
	}

	if (!targetInstance->commit())
	{
		traktor::log::error << L"Failed, unable to commit target instance," << Endl;
		return false;
	}

	return true;
}

/* Migrate instances sequentially.
 *
 * \param targetGroup Target group
 * \param sourceGroup Source group
 * \param groupIndex Index of current group.
 * \param groupCount Number of groups to migrate.
 * \return True if successful.
 */
bool migrateGroup(db::Group* targetGroup, db::Group* sourceGroup, const DateTime& modifiedSince, int32_t& groupIndex, int32_t groupCount)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	traktor::log::info << IncreaseIndent;

	log::info << L":" << groupIndex << L":" << groupCount << Endl;

	RefArray< db::Instance > childInstances;
	sourceGroup->getChildInstances(childInstances);

	for (auto childInstance : childInstances)
	{
		if (!migrateInstance(childInstance, targetGroup, modifiedSince))
			return false;
	}

	++groupIndex;

	RefArray< db::Group > childGroups;
	sourceGroup->getChildGroups(childGroups);

	for (auto childGroup : childGroups)
	{
		Ref< db::Group > targetChildGroup = targetGroup->getGroup(childGroup->getName());
		if (!targetChildGroup)
		{
			targetChildGroup = targetGroup->createGroup(childGroup->getName());
			if (!targetChildGroup)
				return false;
		}

		if (!migrateGroup(targetChildGroup, childGroup, modifiedSince, groupIndex, groupCount))
			return false;
	}

	return true;
}

/*! Load settings.
 *
 * \param settingsFile File path to settings.
 * \return Settings group.
 */
Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< PropertyGroup > settings;
	Ref< traktor::IStream > file;

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file, settingsFile).readObject< PropertyGroup >();
		file->close();
	}

	if (settings)
		return settings;

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file, settingsFile).readObject< PropertyGroup >();
		file->close();
	}

	return settings;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	Ref< traktor::IStream > logFile;

	if (!cmdLine.hasOption('s', L"settings") && cmdLine.getCount() < 2)
	{
		traktor::log::info << L"Database Migration Tool; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;
		traktor::log::info << L"Usage: Traktor.Database.Migrate.App [source database] [destination database] (module)*" << Endl;
		traktor::log::info << L"       Traktor.Database.Migrate.App -s|-settings=[settings]" << Endl;
		traktor::log::info << L"       -s|-settings    Settings (default \"Traktor.Editor\")" << Endl;
		traktor::log::info << L"       -f|-full        Perform full migration regardless if target instance is up-to-date." << Endl;
		traktor::log::info << L"       -l|-log=logfile Save log file" << Endl;
		return 0;
	}

	bool verbose = cmdLine.hasOption('v', L"verbose");

	if (verbose)
		traktor::log::info << L"Database Migration Tool; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	std::wstring sourceCs;
	std::wstring destinationCs;

	// Save log file.
	if (cmdLine.hasOption('l', L"log"))
	{
		std::wstring logPath = cmdLine.getOption('l', L"log").getString();
		if ((logFile = FileSystem::getInstance().open(logPath, File::FmWrite)) != 0)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logStreamTarget = new LogStreamTarget(logStream);

			traktor::log::info   .setGlobalTarget(new LogRedirectTarget(logStreamTarget, traktor::log::info   .getGlobalTarget()));
			traktor::log::warning.setGlobalTarget(new LogRedirectTarget(logStreamTarget, traktor::log::warning.getGlobalTarget()));
			traktor::log::error  .setGlobalTarget(new LogRedirectTarget(logStreamTarget, traktor::log::error  .getGlobalTarget()));

			if (verbose)
				traktor::log::info << L"Log file \"" << logPath << L"\" created." << Endl;
		}
		else
			traktor::log::error << L"Unable to create log file; logging only to std pipes." << Endl;
	}

	// Either read configuration from settings file or from command line.
	if (cmdLine.hasOption('s', L"settings"))
	{
		std::wstring settingsFile = cmdLine.getOption('s', L"settings").getString();

		Ref< PropertyGroup > settings = loadSettings(settingsFile);
		if (!settings)
		{
			traktor::log::error << L"Unable to load migrate settings \"" << settingsFile << L"\"." << Endl;
			return 1;
		}

		auto modulePaths = settings->getProperty< SmallSet< std::wstring > >(L"Migrate.ModulePaths");
		auto modules = settings->getProperty< SmallSet< std::wstring > >(L"Migrate.Modules");

		std::vector< Path > modulePathsFlatten(modulePaths.begin(), modulePaths.end());
		for (const auto& module : modules)
		{
			Library library;
			if (!library.open(module, modulePathsFlatten, true))
			{
				traktor::log::error << L"Unable to load module \"" << module << L"\"." << Endl;
				return 2;
			}
			library.detach();
		}

		sourceCs = settings->getProperty< std::wstring >(L"Migrate.SourceDatabase");
		destinationCs = settings->getProperty< std::wstring >(L"Migrate.OutputDatabase");
	}
	else
	{
		for (size_t i = 2; i < cmdLine.getCount(); ++i)
		{
			Library library;
			if (!library.open(cmdLine.getString(i)))
			{
				traktor::log::error << L"Unable to load module \"" << cmdLine.getString(i) << L"\"." << Endl;
				return 2;
			}
			library.detach();
		}

		sourceCs = cmdLine.getString(0);
		destinationCs = cmdLine.getString(1);
	}

	// Open databases.
	if (verbose)
		traktor::log::info << L"Opening source database \"" << sourceCs << L"\"..." << Endl;

	Ref< db::Database > sourceDb = new db::Database();
	if (!sourceDb->open(sourceCs))
	{
		traktor::log::error << L"Unable to open source database \"" << sourceCs << L"\"." << Endl;
		return 3;
	}

	if (verbose)
		traktor::log::info << L"Opening destination database \"" << destinationCs << L"\"..." << Endl;

	// Get date when output database was last written.
	DateTime modifiedSince;
	if (!cmdLine.hasOption('f', L"full"))
	{
		std::wstring destinationFileName = db::ConnectionString(destinationCs).get(L"fileName");
		if (!destinationFileName.empty())
		{
			Ref< File > destinationFile = FileSystem::getInstance().get(destinationFileName);
			if (destinationFile)
				modifiedSince = destinationFile->getLastWriteTime();
		}
	}

	Ref< db::Database > destinationDb = new db::Database();
	if (!destinationDb->open(destinationCs))
	{
		if (!destinationDb->create(destinationCs))
		{
			traktor::log::error << L"Unable to open nor create destination database \"" << destinationCs << L"\"." << Endl;
			return 4;
		}
	}

	// Begin migration of instances.
	Ref< db::Group > sourceGroup = sourceDb->getRootGroup();
	Ref< db::Group > targetGroup = destinationDb->getRootGroup();
	if (sourceGroup && targetGroup)
	{
		if (verbose)
			traktor::log::info << L"Migration begin, counting groups..." << Endl;

		// Count number of groups; quicker than number of instances but
		// should be sufficient for progress information.
		int32_t groupIndex = 0;
		int32_t groupCount = countGroups(sourceGroup);

		if (verbose)
			traktor::log::info << L"Migrating " << groupCount << L" group(s)..." << Endl;

		if (!migrateGroup(targetGroup, sourceGroup, modifiedSince, groupIndex, groupCount))
			return 5;
	}

	if (verbose)
		traktor::log::info << L"Migration complete." << Endl;

	// Close database connections.
	destinationDb->close();
	sourceDb->close();

	// Close log file if being recorded.
	if (logFile)
	{
		traktor::log::info.setBuffer(nullptr);
		traktor::log::warning.setBuffer(nullptr);
		traktor::log::error.setBuffer(nullptr);
		traktor::log::debug.setBuffer(nullptr);

		logFile->close();
		logFile = nullptr;
	}

	return 0;
}
