/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/FileOutputStream.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StreamCopy.h"
#include "Core/Io/Utf8Encoding.h"
#include "Core/Library/Library.h"
#include "Core/Log/Log.h"
#include "Core/Misc/CommandLine.h"
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

class LogStreamTarget : public ILogTarget
{
public:
	LogStreamTarget(OutputStream* stream)
	:	m_stream(stream)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		(*m_stream) << str << Endl;
	}

private:
	Ref< OutputStream > m_stream;
};

class LogDualTarget : public ILogTarget
{
public:
	LogDualTarget(ILogTarget* target1, ILogTarget* target2)
	:	m_target1(target1)
	,	m_target2(target2)
	{
	}

	virtual void log(uint32_t threadId, int32_t level, const std::wstring& str) T_OVERRIDE T_FINAL
	{
		m_target1->log(threadId, level, str);
		m_target2->log(threadId, level, str);
	}

private:
	Ref< ILogTarget > m_target1;
	Ref< ILogTarget > m_target2;
};

int32_t recursiveCountGroups(db::Group* group)
{
	RefArray< db::Group > childGroups;
	group->getChildGroups(childGroups);

	int32_t localCount = 1;
	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		T_ASSERT (*i);
		localCount += recursiveCountGroups(*i);
	}

	return localCount;
}

bool recursiveConvertInstances(db::Group* targetGroup, db::Group* sourceGroup, int32_t& groupIndex, int32_t groupCount)
{
	T_ANONYMOUS_VAR(ScopeIndent)(log::info);
	traktor::log::info << IncreaseIndent;

	log::info << L":" << groupIndex << L":" << groupCount << Endl;

	RefArray< db::Instance > childInstances;
	sourceGroup->getChildInstances(childInstances);

	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		Ref< db::Instance > sourceInstance = *i;
		T_ASSERT (sourceInstance);

		traktor::log::info << L"Converting \"" << sourceInstance->getName() << L"\"..." << Endl;

		Ref< ISerializable > sourceObject = sourceInstance->getObject();
		if (!sourceObject)
		{
			traktor::log::error << L"Failed, unable to get source object" << Endl;
			return false;
		}

		Guid sourceGuid = sourceInstance->getGuid();
		Ref< db::Instance > targetInstance = targetGroup->createInstance(sourceInstance->getName(), db::CifReplaceExisting, &sourceGuid);
		if (!targetInstance)
		{
			traktor::log::error << L"Failed, unable to create target instance" << Endl;
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
				traktor::log::error << L"Failed, unable to open source stream" << Endl;
				return false;
			}

			Ref< IStream > targetStream = targetInstance->writeData(*j);
			if (!targetStream)
			{
				traktor::log::error << L"Failed, unable to open target stream" << Endl;
				return false;
			}

			if (!StreamCopy(targetStream, sourceStream).execute())
			{
				traktor::log::error << L"Failed, unable to copy data" << Endl;
				return false;
			}

			targetStream->close();
			sourceStream->close();
		}

		if (!targetInstance->commit())
		{
			traktor::log::error << L"Failed, unable to commit target instance" << Endl;
			return false;
		}
	}

	++groupIndex;

	RefArray< db::Group > childGroups;
	sourceGroup->getChildGroups(childGroups);

	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< db::Group > sourceChildGroup = *i;
		T_ASSERT (sourceChildGroup);

		traktor::log::info << L"Creating group \"" << sourceChildGroup->getName() << L"\"..." << Endl;

		Ref< db::Group > targetChildGroup = targetGroup->getGroup(sourceChildGroup->getName());
		if (!targetChildGroup)
		{
			targetChildGroup = targetGroup->createGroup(sourceChildGroup->getName());
			if (!targetChildGroup)
			{
				traktor::log::error << L"Failed, unable to create target group" << Endl;
				return false;
			}
		}

		if (!recursiveConvertInstances(targetChildGroup, sourceChildGroup, groupIndex, groupCount))
			return false;
	}

	return true;
}

Ref< PropertyGroup > loadSettings(const std::wstring& settingsFile)
{
	Ref< PropertyGroup > settings;
	Ref< traktor::IStream > file;

	std::wstring globalConfig = settingsFile + L".config";
	std::wstring userConfig = settingsFile + L"." + OS::getInstance().getCurrentUser() + L".config";

	if ((file = FileSystem::getInstance().open(userConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	if (settings)
	{
		traktor::log::info << L"Using configuration \"" << userConfig << L"\"" << Endl;
		return settings;
	}

	if ((file = FileSystem::getInstance().open(globalConfig, File::FmRead)) != 0)
	{
		settings = xml::XmlDeserializer(file).readObject< PropertyGroup >();
		file->close();
	}

	if (settings)
	{
		traktor::log::info << L"Using configuration \"" << globalConfig << L"\"" << Endl;
		return settings;
	}

	return 0;
}

}

int main(int argc, const char** argv)
{
	CommandLine cmdLine(argc, argv);
	Ref< traktor::IStream > logFile;

	traktor::log::info << L"Database Migration Tool; Built '" << mbstows(__TIME__) << L" - " << mbstows(__DATE__) << L"'" << Endl;

	if (!cmdLine.hasOption('s', L"settings") && cmdLine.getCount() < 2)
	{
		traktor::log::info << L"Usage: Traktor.Database.Migrate.App [source database] [destination database] (module)*" << Endl;
		traktor::log::info << L"       Traktor.Database.Migrate.App -s|-settings=[settings]" << Endl;
		traktor::log::info << L"       -s|-settings    Settings (default \"Traktor.Editor\")" << Endl;
		traktor::log::info << L"       -l|-log=logfile Save log file" << Endl;
		return 0;
	}

	std::wstring sourceCs;
	std::wstring destinationCs;

	if (cmdLine.hasOption('l', L"log"))
	{
		std::wstring logPath = cmdLine.getOption('l', L"log").getString();
		if ((logFile = FileSystem::getInstance().open(logPath, File::FmWrite)) != 0)
		{
			Ref< FileOutputStream > logStream = new FileOutputStream(logFile, new Utf8Encoding());
			Ref< LogStreamTarget > logStreamTarget = new LogStreamTarget(logStream);

			traktor::log::info   .setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::info   .getGlobalTarget()));
			traktor::log::warning.setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::warning.getGlobalTarget()));
			traktor::log::error  .setGlobalTarget(new LogDualTarget(logStreamTarget, traktor::log::error  .getGlobalTarget()));

			traktor::log::info << L"Log file \"" << logPath << L"\" created" << Endl;
		}
		else
			traktor::log::error << L"Unable to create log file; logging only to std pipes" << Endl;
	}

	if (cmdLine.hasOption('s', L"settings"))
	{
		std::wstring settingsFile = cmdLine.getOption('s', L"settings").getString();

		Ref< PropertyGroup > settings = loadSettings(settingsFile);
		if (!settings)
		{
			traktor::log::error << L"Unable to load migrate settings \"" << settingsFile << L"\"" << Endl;
			return 1;
		}

		std::set< std::wstring > modules = settings->getProperty< std::set< std::wstring > >(L"Migrate.Modules");
		for (std::set< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
		{
			Library library;
			if (!library.open(*i))
			{
				traktor::log::error << L"Unable to load module \"" << *i << L"\"" << Endl;
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
				traktor::log::error << L"Unable to load module \"" << cmdLine.getString(i) << L"\"" << Endl;
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
		traktor::log::error << L"Unable to open source database \"" << sourceCs << L"\"" << Endl;
		return 3;
	}

	Ref< db::Database > destinationDb = new db::Database();
	if (!destinationDb->create(destinationCs))
	{
		traktor::log::error << L"Unable to create destination database \"" << destinationCs << L"\"" << Endl;
		return 4;
	}

	traktor::log::info << L"Migration begin" << Endl;

	Ref< db::Group > sourceGroup = sourceDb->getRootGroup();
	Ref< db::Group > targetGroup = destinationDb->getRootGroup();
	if (sourceGroup && targetGroup)
	{
		// Count number of groups; quicker than number of instances but
		// should be sufficient for progress information.
		int32_t groupIndex = 0;
		int32_t groupCount = recursiveCountGroups(sourceGroup);

		if (!recursiveConvertInstances(targetGroup, sourceGroup, groupIndex, groupCount))
			return 5;
	}

	traktor::log::info << L"Migration complete" << Endl;

	destinationDb->close();
	sourceDb->close();

	if (logFile)
	{
		traktor::log::info.setBuffer(0);
		traktor::log::warning.setBuffer(0);
		traktor::log::error.setBuffer(0);
		traktor::log::debug.setBuffer(0);

		logFile->close();
		logFile = 0;
	}

	return 0;
}
