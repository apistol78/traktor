/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
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
#include "Core/System/ResolveEnv.h"
#include "Database/ConnectionString.h"
#include "Database/Database.h"
#include "Runtime/Editor/Deploy/DeployTargetAction.h"
#include "Runtime/Editor/Deploy/Feature.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor::runtime
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
		const auto ss = PropertyStringArray::get(valueStringArray);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else if (const PropertyStringSet* valueStringSet = dynamic_type_cast< const PropertyStringSet* >(value))
	{
		const auto ss = PropertyStringSet::get(valueStringSet);
		return implode(ss.begin(), ss.end(), L"\n");
	}
	else
		return L"";
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.DeployTargetAction", DeployTargetAction, ITargetAction)

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
	for (const auto& featureId : featureIds)
	{
		Ref< const Feature > feature = m_database->getObjectReadOnly< Feature >(featureId);
		if (!feature)
		{
			log::warning << L"Unable to get feature \"" << featureId.format() << L"\"; feature skipped." << Endl;
			continue;
		}
		features.push_back(feature);
	}

	features.sort(FeaturePriorityPred());

	// Insert target's features into pipeline configuration. Also generate a set of files to deploy.
	for (auto feature : features)
	{
		Ref< const PropertyGroup > runtimeProperties = feature->getRuntimeProperties();
		if (runtimeProperties)
			applicationConfiguration = applicationConfiguration->merge(runtimeProperties, PropertyGroup::MmJoin);

		const Feature::Platform* fp = feature->getPlatform(m_targetConfiguration->getPlatform());
		if (fp)
		{
			if (fp->deploy)
				deploy = deploy->merge(fp->deploy, PropertyGroup::MmJoin);
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
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.Database", remoteCs.format());
	applicationConfiguration->setProperty< PropertyBoolean >(L"Runtime.DatabaseThread", true);

	// Modify configuration to connect to embedded target manager.
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.TargetManager/Host", m_editorHost);
	applicationConfiguration->setProperty< PropertyInteger >(L"Runtime.TargetManager/Port", m_targetManagerPort);
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.TargetManager/Id", m_targetManagerId.format());

	// Append target guid;s to application configuration.
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.Root", m_targetConfiguration->getRoot().format());
	applicationConfiguration->setProperty< PropertyString >(L"Runtime.Startup", m_targetConfiguration->getStartup().format());
	applicationConfiguration->setProperty< PropertyString >(L"Input.Default", m_targetConfiguration->getDefaultInput().format());
	applicationConfiguration->setProperty< PropertyString >(L"Online.Config", m_targetConfiguration->getOnlineConfig().format());

	// Append application title.
	applicationConfiguration->setProperty< PropertyString >(L"Render.Title", m_targetName);
	applicationConfiguration->setProperty< PropertyBoolean >(L"Render.UseProgramCache", false);

	// Append verbose resource manager.
	if (m_globalSettings->getProperty< bool >(L"Resource.Verbose", false))
		applicationConfiguration->setProperty< PropertyBoolean >(L"Resource.Verbose", true);

	// Append tweaks.
	if (m_tweakSettings)
		applicationConfiguration = applicationConfiguration->merge(m_tweakSettings, PropertyGroup::MmJoin);

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
	const auto runtimeModules = applicationConfiguration->getProperty< SmallSet< std::wstring > >(L"Runtime.Modules");

	// Launch deploy tool to ensure platform is ready for launch.
	const Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	Ref< Environment > env = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathName());
#else
	env->set(L"DEPLOY_PROJECT_ROOT", projectRoot.getPathNameNoVolume());
#endif
	env->set(L"DEPLOY_PROJECT_NAME", m_targetName);
	env->set(L"DEPLOY_PROJECT_IDENTIFIER", m_target->getIdentifier());
	env->set(L"DEPLOY_PROJECT_VERSION", m_target->getVersion());
	env->set(L"DEPLOY_PROJECT_ICON", m_targetConfiguration->getIcon());
	env->set(L"DEPLOY_SYSTEM_ROOT", m_globalSettings->getProperty< std::wstring >(L"Runtime.SystemRoot", L"$(TRAKTOR_HOME)"));
	env->set(L"DEPLOY_TARGET_HOST", m_deployHost);
	env->set(L"DEPLOY_EXECUTABLE", executableFile);
	env->set(L"DEPLOY_MODULES", implode(runtimeModules.begin(), runtimeModules.end(), L" "));
	env->set(L"DEPLOY_OUTPUT_PATH", m_outputPath);
	env->set(L"DEPLOY_DEBUG", m_globalSettings->getProperty< bool >(L"Runtime.UseDebugBinaries", false) ? L"YES" : L"");
	env->set(L"DEPLOY_STATIC_LINK", m_globalSettings->getProperty< bool >(L"Runtime.StaticallyLinked", false) ? L"YES" : L"");

	env->set(L"DEPLOY_ANDROID_HOME", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidHome", L"$(ANDROID_HOME)"), 0));
	env->set(L"DEPLOY_ANDROID_NDK_ROOT", resolveEnv(m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidNdkRoot", L"$(ANDROID_NDK_ROOT)"), 0));
	env->set(L"DEPLOY_ANDROID_TOOLCHAIN", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidToolchain", L""));
	env->set(L"DEPLOY_ANDROID_APILEVEL", m_globalSettings->getProperty< std::wstring >(L"Runtime.AndroidApiLevel", L""));

	// Flatten feature deploy variables.
	for (const auto& value : deploy->getValues())
		env->set(value.first, implodePropertyValue(value.second));

	// Merge tool environment variables.
	const DeployTool& deployTool = platform->getDeployTool();
	env->insert(deployTool.getEnvironment());

	// Merge all feature environment variables.
	for (auto feature : features)
		env->insert(feature->getEnvironment());

	// Merge settings environment variables.
	Ref< PropertyGroup > settingsEnvironment = m_globalSettings->getProperty< PropertyGroup >(L"Runtime.Environment");
	if (settingsEnvironment)
	{
		for (const auto pair : settingsEnvironment->getValues())
		{
			const PropertyString* value = dynamic_type_cast< PropertyString* >(pair.second);
			if (value)
			{
				env->set(
					pair.first,
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
		OS::EfRedirectStdIO | OS::EfMute
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
		Ref< IStream > pipe;
		IProcess::WaitPipeResult result = process->waitPipeStream(100, pipe);
		if (result == IProcess::Ready && pipe != nullptr)
		{
			if (pipe == process->getPipeStream(IProcess::SpStdOut))
			{
				PipeReader::Result result;
				while ((result = stdOutReader.readLine(str)) == PipeReader::RtOk)
				{
					if (progressListener)
						progressListener->notifyLog(str);
					else
						log::info << str << Endl;
				}
			}
			else if (pipe == process->getPipeStream(IProcess::SpStdErr))
			{
				PipeReader::Result result;
				while ((result = stdErrReader.readLine(str)) == PipeReader::RtOk)
				{
					if (progressListener)
						progressListener->notifyLog(str);
					else
						log::error << str << Endl;
				}
			}
		}
		else if (result == IProcess::Terminated)
			break;
	}

	const int32_t exitCode = process->exitCode();
	//if (exitCode != 0)
	//	log::error << L"Process \"" << deployTool.getExecutable() << L" deploy\" failed with exit code " << exitCode << L"." << Endl;

	if (progressListener)
		progressListener->notifyTargetActionProgress(2, 2);

	return exitCode == 0;
}

}
