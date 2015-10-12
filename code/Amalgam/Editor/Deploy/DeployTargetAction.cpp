#include "Amalgam/Editor/Deploy/DeployTargetAction.h"
#include "Amalgam/Editor/Deploy/Feature.h"
#include "Amalgam/Editor/Deploy/Platform.h"
#include "Amalgam/Editor/Deploy/Target.h"
#include "Amalgam/Editor/Deploy/TargetConfiguration.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyObject.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/System/Environment.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Core/System/PipeReader.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

struct FeaturePriorityPred
{
	bool operator () (const Feature* l, const Feature* r) const
	{
		return l->getPriority() < r->getPriority();
	}
};

std::wstring implodePropertyValue(const IPropertyValue* value)
{
	if (const PropertyString* valueString = dynamic_type_cast< const PropertyString* >(value))
		return PropertyString::get(valueString);
	else if (const PropertyStringArray* valueStringArray = dynamic_type_cast< const PropertyStringArray* >(value))
	{
		std::vector< std::wstring > ss = PropertyStringArray::get(valueStringArray);
		return implode(ss.begin(), ss.end(), L" ");
	}
	else if (const PropertyStringSet* valueStringSet = dynamic_type_cast< const PropertyStringSet* >(value))
	{
		std::set< std::wstring > ss = PropertyStringSet::get(valueStringSet);
		return implode(ss.begin(), ss.end(), L" ");
	}
	else
		return L"";
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.DeployTargetAction", DeployTargetAction, ITargetAction)

DeployTargetAction::DeployTargetAction(
	db::Database* database,
	const PropertyGroup* globalSettings,
	const std::wstring& targetName,
	const Target* target,
	const TargetConfiguration* targetConfiguration,
	const std::wstring& editorHost,
	const std::wstring& deployHost,
	uint16_t databasePort,
	const std::wstring& databaseName,
	uint16_t targetManagerPort,
	const Guid& targetManagerId,
	const std::wstring& outputPath,
	const PropertyGroup* tweakSettings
)
:	m_database(database)
,	m_globalSettings(globalSettings)
,	m_targetName(targetName)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_editorHost(editorHost)
,	m_deployHost(deployHost)
,	m_databasePort(databasePort)
,	m_databaseName(databaseName)
,	m_targetManagerPort(targetManagerPort)
,	m_targetManagerId(targetManagerId)
,	m_outputPath(outputPath)
,	m_tweakSettings(tweakSettings)
{
}

bool DeployTargetAction::execute(IProgressListener* progressListener)
{
	Ref< PropertyGroup > deploy = new PropertyGroup();
	std::wstring executableFile;

	// Get platform description object from database.
	Ref< Platform > platform = m_database->getObjectReadOnly< Platform >(m_targetConfiguration->getPlatform());
	if (!platform)
	{
		log::error << L"Unable to get platform object" << Endl;
		return false;
	}

	if (progressListener)
		progressListener->notifyTargetActionProgress(1, 2);

	// Create target application configuration.
	Ref< PropertyGroup > applicationConfiguration = new PropertyGroup();

	// Get features; sorted by priority.
	const std::list< Guid >& featureIds = m_targetConfiguration->getFeatures();

	RefArray< const Feature > features;
	for (std::list< Guid >::const_iterator i = featureIds.begin(); i != featureIds.end(); ++i)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(*i);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << i->format() << L"\"; feature skipped." << Endl;
			continue;
		}
		features.push_back(feature);
	}

	features.sort(FeaturePriorityPred());

	// Insert target's features into pipeline configuration. Also generate a set of files to deploy.
	for (RefArray< const Feature >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		const Feature* feature = *i;
		T_ASSERT (feature);

		Ref< const PropertyGroup > runtimeProperties = feature->getRuntimeProperties();
		if (runtimeProperties)
			applicationConfiguration = applicationConfiguration->mergeJoin(runtimeProperties);

		const Feature::Platform* fp = feature->getPlatform(m_targetConfiguration->getPlatform());
		if (fp)
		{
			deploy = deploy->mergeJoin(fp->deploy);
			if (!fp->executableFile.empty())
				executableFile = fp->executableFile;
		}
		else
			log::warning << L"Feature \"" << feature->getDescription() << L"\" doesn't support selected platform." << Endl;
	}

	// Modify configuration to connect to embedded database server.
	db::ConnectionString remoteCs;
	remoteCs.set(L"provider", L"traktor.db.RemoteDatabase");
	remoteCs.set(L"host", m_editorHost + L":" + toString(m_databasePort));
	remoteCs.set(L"database", m_databaseName);
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Database", remoteCs.format());
	applicationConfiguration->setProperty< PropertyBoolean >(L"Amalgam.DatabaseThread", true);
	
	// Modify configuration to connect to embedded target manager.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.TargetManager/Host", m_editorHost);
	applicationConfiguration->setProperty< PropertyInteger >(L"Amalgam.TargetManager/Port", m_targetManagerPort);
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.TargetManager/Id", m_targetManagerId.format());

	// Append target guid;s to application configuration.
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Root", m_targetConfiguration->getRoot().format());
	applicationConfiguration->setProperty< PropertyString >(L"Amalgam.Startup", m_targetConfiguration->getStartup().format());
	applicationConfiguration->setProperty< PropertyString >(L"Input.Default", m_targetConfiguration->getDefaultInput().format());
	applicationConfiguration->setProperty< PropertyString >(L"Online.Config", m_targetConfiguration->getOnlineConfig().format());

	// Append application title.
	applicationConfiguration->setProperty< PropertyString >(L"Render.Title", m_targetName);

	// Append tweaks.
	if (m_tweakSettings)
		applicationConfiguration = applicationConfiguration->mergeJoin(m_tweakSettings);

	// Write generated application configuration in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
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

	// Get list of used modules from application configuration.
	std::set< std::wstring > runtimeModules = applicationConfiguration->getProperty< PropertyStringSet >(L"Amalgam.Modules");

	// Launch deploy tool to ensure platform is ready for launch.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	Ref< Environment > env = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathName());
#else
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathNameNoVolume());
#endif
	env->set(L"DEPLOY_PROJECT_NAME", m_targetName);
	env->set(L"DEPLOY_PROJECT_IDENTIFIER", m_target->getIdentifier());
	env->set(L"DEPLOY_PROJECT_ICON", m_targetConfiguration->getIcon());
	env->set(L"DEPLOY_SYSTEM_ROOT", m_globalSettings->getProperty< PropertyString >(L"Amalgam.SystemRoot", L"$(TRAKTOR_HOME)"));
	env->set(L"DEPLOY_TARGET_HOST", m_deployHost);
	env->set(L"DEPLOY_EXECUTABLE", executableFile);
	env->set(L"DEPLOY_MODULES", implode(runtimeModules.begin(), runtimeModules.end(), L" "));
	env->set(L"DEPLOY_OUTPUT_PATH", m_outputPath);
	env->set(L"DEPLOY_DEBUG", m_globalSettings->getProperty< PropertyBoolean >(L"Amalgam.UseDebugBinaries", false) ? L"YES" : L"");
	env->set(L"DEPLOY_STATIC_LINK", m_globalSettings->getProperty< PropertyBoolean >(L"Amalgam.StaticallyLinked", false) ? L"YES" : L"");

	// Flatten feature deploy variables.
	const std::map< std::wstring, Ref< IPropertyValue > >& values = deploy->getValues();
	for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		env->set(i->first, implodePropertyValue(i->second));

	// Merge tool environment variables.
	const DeployTool& deployTool = platform->getDeployTool();
	env->insert(deployTool.getEnvironment());

	// Merge all feature environment variables.
	for (RefArray< const Feature >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		const Feature* feature = *i;
		T_ASSERT (feature);

		env->insert(feature->getEnvironment());
	}

	// Merge settings environment variables.
	Ref< PropertyGroup > settingsEnvironment = m_globalSettings->getProperty< PropertyGroup >(L"Amalgam.Environment");
	if (settingsEnvironment)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = settingsEnvironment->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				env->set(
					i->first,
					PropertyString::get(value)
				);
			}
		}
	}

	// Launch deploy process.
	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable() + L" deploy",
		m_outputPath,
		env,
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

	std::wstring str;
	for (;;)
	{
		PipeReader::Result result1 = stdOutReader.readLine(str, 10);
		if (result1 == PipeReader::RtOk)
			log::info << str << Endl;

		PipeReader::Result result2 = stdErrReader.readLine(str, 10);
		if (result2 == PipeReader::RtOk)
			log::error << str << Endl;

		if (result1 == PipeReader::RtEnd && result2 == PipeReader::RtEnd)
			break;
	}

	int32_t exitCode = process->exitCode();
	if (exitCode != 0)
		log::error << L"Process failed with exit code " << exitCode << Endl;

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 2);

	return exitCode == 0;
}

	}
}
