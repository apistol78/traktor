#include <list>
#include "Amalgam/Editor/Feature.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/Tool/MigrateTargetAction.h"
#include "Amalgam/Editor/Tool/PipeReader.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.MigrateTargetAction", MigrateTargetAction, ITargetAction)

MigrateTargetAction::MigrateTargetAction(
	db::Database* database,
	const PropertyGroup* globalSettings,
	const std::wstring& targetName,
	const Target* target,
	const TargetConfiguration* targetConfiguration,
	const std::wstring& deployHost,
	const std::wstring& outputPath
)
:	m_database(database)
,	m_globalSettings(globalSettings)
,	m_targetName(targetName)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_deployHost(deployHost)
,	m_outputPath(outputPath)
{
}

bool MigrateTargetAction::execute(IProgressListener* progressListener)
{
	// Get platform description object from database.
	Ref< Platform > platform = m_database->getObjectReadOnly< Platform >(m_targetConfiguration->getPlatform());
	if (!platform)
	{
		log::error << L"Unable to get platform object" << Endl;
		return false;
	}

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 100);

	// Ensure output directory exists.
	if (!FileSystem::getInstance().makeAllDirectories(m_outputPath))
	{
		log::error << L"Unable to create output path \"" << m_outputPath << L"\"" << Endl;
		return false;
	}

	// Set database connection strings.
	db::ConnectionString sourceDatabaseCs(L"provider=traktor.db.LocalDatabase;groupPath=db;binary=true;eventFile=false");
	db::ConnectionString outputDatabaseCs(L"provider=traktor.db.CompactDatabase;fileName=Content.compact;flushAlways=false");
	db::ConnectionString applicationDatabaseCs(L"provider=traktor.db.CompactDatabase;fileName=Content.compact;readOnly=true");

	// Create migration configuration.
	Ref< PropertyGroup > migrateConfiguration = new PropertyGroup();

	// Insert target's features into migrate configuration.
	const std::list< Guid >& features = m_targetConfiguration->getFeatures();
	for (std::list< Guid >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(*i);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << i->format() << L"\"; feature skipped." << Endl;
			continue;
		}

		Ref< const PropertyGroup > migrateProperties = feature->getMigrateProperties();
		if (!migrateProperties)
			continue;

		migrateConfiguration = migrateConfiguration->mergeJoin(migrateProperties);
	}

	migrateConfiguration->setProperty< PropertyString >(L"Migrate.SourceDatabase", sourceDatabaseCs.format());
	migrateConfiguration->setProperty< PropertyString >(L"Migrate.OutputDatabase", outputDatabaseCs.format());

	// Create target application configuration.
	Ref< PropertyGroup > applicationConfiguration = new PropertyGroup();

	// Insert features into runtime configuration.
	for (std::list< Guid >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(*i);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << i->format() << L"\"; feature skipped." << Endl;
			continue;
		}

		Ref< const PropertyGroup > runtimeProperties = feature->getRuntimeProperties();
		if (!runtimeProperties)
			continue;

		applicationConfiguration = applicationConfiguration->mergeJoin(runtimeProperties);
	}

	// Modify configuration to connect to migrated database.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Database", applicationDatabaseCs.format());

	// Append target guid;s to application configuration.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Root", m_targetConfiguration->getRoot().format());
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Startup", m_targetConfiguration->getStartup().format());
	applicationConfiguration->setProperty< PropertyString >(L"Input.Default", m_targetConfiguration->getDefaultInput().format());
	applicationConfiguration->setProperty< PropertyString >(L"Online.Config", m_targetConfiguration->getOnlineConfig().format());

	// Write generated configurations in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
		m_outputPath + L"/Migrate.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(migrateConfiguration);
		file->close();
	}
	else
	{
		log::error << L"Unable to write migrate configuration" << Endl;
		return false;
	}

	file = FileSystem::getInstance().open(
		m_outputPath + L"/Application.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(applicationConfiguration);
		file->close();
	}
	else
	{
		log::error << L"Unable to write application configuration" << Endl;
		return false;
	}

	// Launch migration through deploy tool; set cwd to output directory.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECT_ROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECT_ROOT"] = projectRoot.getPathNameNoVolume();
#endif
	envmap[L"DEPLOY_PROJECT_NAME"] = m_targetName;
	envmap[L"DEPLOY_PROJECT_IDENTIFIER"] = m_target->getIdentifier();
	envmap[L"DEPLOY_PROJECT_ICON"] = m_targetConfiguration->getIcon();
	envmap[L"DEPLOY_TARGET_HOST"] = m_deployHost;
	envmap[L"DEPLOY_EXECUTABLE"] = m_targetConfiguration->getExecutable();
	envmap[L"DEPLOY_OUTPUT_PATH"] = m_outputPath;

	const DeployTool& deployTool = platform->getDeployTool();
	envmap.insert(deployTool.getEnvironment().begin(), deployTool.getEnvironment().end());

	StringOutputStream ss;
	ss << L"migrate";

	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable(),
		ss.str(),
		m_outputPath,
		&envmap,
		true, true, false
	);
	if (!process)
	{
		log::error << L"Failed to launch process \"" << deployTool.getExecutable() << L"\"" << Endl;
		return false;
	}

	PipeReader stdOutReader(
		process->getPipeStream(IProcess::SpStdOut)
	);
	PipeReader stdErrReader(
		process->getPipeStream(IProcess::SpStdErr)
	);

	std::list< std::wstring > errors;
	std::wstring str;

	while (!process->wait(100))
	{
		while (stdOutReader.readLine(str, 10))
		{
			str = trim(str);
			if (str.empty())
				continue;

			if (str[0] == L':')
			{
				std::vector< std::wstring > out;
				if (Split< std::wstring >::any(str, L":", out) == 2)
				{
					int32_t index = parseString< int32_t >(out[0]);
					int32_t count = parseString< int32_t >(out[1]);
					if (count > 0)
					{
						if (progressListener)
							progressListener->notifyTargetActionProgress(2 + (98 * index) / count, 100);
					}
				}
			}
		}

		while (stdErrReader.readLine(str, 10))
		{
			str = trim(str);
			if (!str.empty())
				errors.push_back(str);
		}
	}

	if (!errors.empty())
	{
		log::error << L"Unsuccessful build, error(s):" << Endl;
		for (std::list< std::wstring >::const_iterator i = errors.begin(); i != errors.end(); ++i)
			log::error << L"\t" << *i << Endl;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process failed with exit code " << exitCode << Endl;

	return exitCode == 0;
}

	}
}
