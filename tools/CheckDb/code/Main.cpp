/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <Core/Log/Log.h>
#include <Core/Misc/CommandLine.h>
#include <Database/Database.h>
#include <Database/Group.h>
#include <Database/Instance.h>
#include <Database/Compact/CompactDatabase.h>
#include <Database/Local/LocalDatabase.h>
#include <Database/Remote/Client/RemoteDatabase.h>

using namespace traktor;

std::map< Guid, std::wstring > s_instanceGuids;
uint32_t s_issues = 0;

void recursiveCheckIntegrity(db::Group* sourceGroup)
{
	RefArray< db::Instance > childInstances;
	sourceGroup->getChildInstances(childInstances);

	for (RefArray< db::Instance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
	{
		Ref< db::Instance > sourceInstance = *i;
		T_ASSERT (sourceInstance);

		Guid sourceGuid = sourceInstance->getGuid();
		std::wstring sourcePath = sourceInstance->getPath();

		std::map< Guid, std::wstring >::const_iterator it = s_instanceGuids.find(sourceGuid);
		if (it != s_instanceGuids.end())
		{
			log::error << L"Unique identifier collision detected" << Endl <<
				L"\tin instance \"" << sourcePath << L"\"," << Endl <<
				L"\twith \"" << it->second << L"\"" << Endl;
			s_issues++;
		}
		else
			s_instanceGuids.insert(std::make_pair(
				sourceGuid,
				sourcePath
			));
	}

	RefArray< db::Group > childGroups;
	sourceGroup->getChildGroups(childGroups);

	for (RefArray< db::Group >::iterator i = childGroups.begin(); i != childGroups.end(); ++i)
	{
		Ref< db::Group > sourceChildGroup = *i;
		T_ASSERT (sourceChildGroup);

		recursiveCheckIntegrity(sourceChildGroup);
	}
}

int main(int argc, const char** argv)
{
	T_FORCE_LINK_REF(db::LocalDatabase);
	T_FORCE_LINK_REF(db::RemoteDatabase);
	T_FORCE_LINK_REF(db::CompactDatabase);

	CommandLine cmdLine(argc, argv);
	if (cmdLine.getCount() < 1)
	{
		log::info << L"Usage: CheckDb [database]" << Endl;
		return 1;
	}

	std::wstring sourceCs = cmdLine.getString(0);

	Ref< db::Database > sourceDb = new db::Database();
	if (!sourceDb->open(sourceCs))
	{
		log::error << L"Unable to open database \"" << sourceCs << L"\"" << Endl;
		return 1;
	}

	Ref< db::Group > sourceGroup = sourceDb->getRootGroup();
	if (sourceGroup)
		recursiveCheckIntegrity(sourceGroup);

	if (s_issues)
		log::info << L"Found " << s_issues << L" issue(s)" << Endl;
	else
		log::info << L"Integrity check successful" << Endl;

	sourceDb->close();

	return (s_issues == 0) ? 0 : 2;
}
