#include "Amalgam/Editor/Feature.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/Tool/BuildTargetAction.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
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

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.BuildTargetAction", BuildTargetAction, ITargetAction)

BuildTargetAction::BuildTargetAction(
	db::Database* database,
	const PropertyGroup* globalSettings,
	const PropertyGroup* defaultPipelineSettings,
	const Target* target,
	const TargetConfiguration* targetConfiguration,
	const std::wstring& outputPath
)
:	m_database(database)
,	m_globalSettings(globalSettings)
,	m_defaultPipelineSettings(defaultPipelineSettings)
,	m_target(target)
,	m_targetConfiguration(targetConfiguration)
,	m_outputPath(outputPath)
{
}

bool BuildTargetAction::execute(IProgressListener* progressListener)
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

	// Create target pipeline configuration.
	Ref< PropertyGroup > pipelineConfiguration = new PropertyGroup();
	if (m_defaultPipelineSettings)
		pipelineConfiguration = DeepClone(m_defaultPipelineSettings).create< PropertyGroup >();

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

	// Insert target's features into pipeline configuration.
	for (RefArray< const Feature >::const_iterator i = features.begin(); i != features.end(); ++i)
	{
		const Feature* feature = *i;
		T_ASSERT (feature);

		Ref< const PropertyGroup > pipelineProperties = feature->getPipelineProperties();
		if (!pipelineProperties)
			continue;

		pipelineConfiguration = pipelineConfiguration->mergeJoin(pipelineProperties);
	}

	// Merge pipeline cache configuration from global configuration.
	bool inheritCache = m_globalSettings->getProperty< PropertyBoolean >(L"Amalgam.InheritCache", true);
	if (inheritCache)
	{
		bool fileCacheEnable = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.FileCache", false);
		if (fileCacheEnable)
		{
			std::wstring fileCachePath = m_globalSettings->getProperty< PropertyString >(L"Pipeline.FileCache.Path", L"");
			fileCachePath = FileSystem::getInstance().getAbsolutePath(fileCachePath).getPathName();
			if (!fileCachePath.empty())
			{
				bool fileCacheRead = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", false);
				bool fileCacheWrite = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", false);

				pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.FileCache", true);
				pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.FileCache.Path", fileCachePath);
				pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Read", fileCacheRead);
				pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.FileCache.Write", fileCacheWrite);
			}
		}

		bool memCachedEnable = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.MemCached", false);
		if (memCachedEnable)
		{
			std::wstring memCachedHost = m_globalSettings->getProperty< PropertyString >(L"Pipeline.MemCached.Host", L"");
			int32_t memCachedPort = m_globalSettings->getProperty< PropertyInteger >(L"Pipeline.MemCached.Port", 0);
			bool memCachedRead = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", false);
			bool memCachedWrite = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", false);

			pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.MemCached", true);
			pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.MemCached.Host", memCachedHost);
			pipelineConfiguration->setProperty< PropertyInteger >(L"Pipeline.MemCached.Port", memCachedPort);
			pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Read", memCachedRead);
			pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.MemCached.Write", memCachedWrite);
		}
	}

	// Use instance cache path from global configuration.
	std::wstring cachePath = m_globalSettings->getProperty< PropertyString >(L"Pipeline.CachePath");
	cachePath = FileSystem::getInstance().getAbsolutePath(cachePath).getPathName();
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.CachePath", cachePath);

	// Merge threaded build configuration from global configuration.
	bool buildThreads = m_globalSettings->getProperty< PropertyBoolean >(L"Pipeline.BuildThreads", true);
	pipelineConfiguration->setProperty< PropertyBoolean >(L"Pipeline.BuildThreads", buildThreads);

	// Set database connection strings.
	db::ConnectionString sourceDatabaseCs = m_globalSettings->getProperty< PropertyString >(L"Editor.SourceDatabase");
	db::ConnectionString outputDatabaseCs(L"provider=traktor.db.LocalDatabase;groupPath=" + FileSystem::getInstance().getAbsolutePath(m_outputPath + L"/db").getPathName() + L";binary=true");

	if (sourceDatabaseCs.have(L"groupPath"))
	{
		Path groupPath = FileSystem::getInstance().getAbsolutePath(sourceDatabaseCs.get(L"groupPath"));
		sourceDatabaseCs.set(L"groupPath", groupPath.getPathName());
	}

	sourceDatabaseCs.set(L"fileStore", L"");

	pipelineConfiguration->setProperty< PropertyString >(L"Editor.SourceDatabase", sourceDatabaseCs.format());
	pipelineConfiguration->setProperty< PropertyString >(L"Editor.OutputDatabase", outputDatabaseCs.format());

	// Set asset path.
	Path assetPath = m_globalSettings->getProperty< PropertyString >(L"Pipeline.AssetPath");
	assetPath = FileSystem::getInstance().getAbsolutePath(assetPath);
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.AssetPath", assetPath.getPathName());

	// Insert mesh pipeline's material templates.
	const IPropertyValue* materialTemplates = m_globalSettings->getProperty(L"MeshPipeline.MaterialTemplates");
	if (materialTemplates)
		pipelineConfiguration->setProperty(L"MeshPipeline.MaterialTemplates", materialTemplates->clone());

	// Ensure output directory exists.
	if (!FileSystem::getInstance().makeAllDirectories(m_outputPath))
	{
		log::error << L"Unable to create output path \"" << m_outputPath << L"\"" << Endl;
		return false;
	}

	// Write generated pipeline configuration in output directory.
	Ref< IStream > file = FileSystem::getInstance().open(
		m_outputPath + L"/Pipeline.config",
		File::FmWrite
	);
	if (file)
	{
		xml::XmlSerializer(file).writeObject(pipelineConfiguration);
		file->close();
	}
	else
	{
		log::error << L"Unable to write pipeline configuration" << Endl;
		return false;
	}

	// Launch pipeline through deploy tool; set cwd to output directory.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECT_ROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECT_ROOT"] = projectRoot.getPathNameNoVolume();
#endif
	envmap[L"DEPLOY_SYSTEM_ROOT"] = m_targetConfiguration->getSystemRoot();
	envmap[L"DEPLOY_OUTPUT_PATH"] = m_outputPath;
	envmap[L"DEPLOY_CERTIFICATE"] = m_globalSettings->getProperty< PropertyString >(L"Amalgam.Certificate", L"");

	const DeployTool& deployTool = platform->getDeployTool();
	envmap.insert(deployTool.getEnvironment().begin(), deployTool.getEnvironment().end());

	StringOutputStream ss;
	ss << deployTool.getExecutable() << L" build";

	Guid root = m_targetConfiguration->getRoot();
	if (root.isValid() && !root.isNull())
		ss << L" " << root.format();

	Guid startup = m_targetConfiguration->getStartup();
	if (startup.isValid() && !startup.isNull())
		ss << L" " << startup.format();

	Guid defaultInput = m_targetConfiguration->getDefaultInput();
	if (defaultInput.isValid() && !defaultInput.isNull())
		ss << L" " << defaultInput.format();

	Guid onlineConfig = m_targetConfiguration->getOnlineConfig();
	if (onlineConfig.isValid() && !onlineConfig.isNull())
		ss << L" " << onlineConfig.format();

	Ref< IProcess > process = OS::getInstance().execute(
		ss.str(),
		m_outputPath,
		&envmap,
#if defined(_DEBUG)
		false, false, false
#else
		true, true, false
#endif
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

	do
	{
		while (stdOutReader.readLine(str, 100))
		{
			std::wstring tmp = trim(str);
			if (!tmp.empty() && tmp[0] == L':')
			{
				std::vector< std::wstring > out;
				if (Split< std::wstring >::any(tmp, L":", out) == 2)
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
			else
				log::info << str << Endl;
		}

		while (stdErrReader.readLine(str, 100))
		{
			str = trim(str);
			if (!str.empty())
			{
				log::error << str << Endl;
				errors.push_back(str);
			}
		}
	}
	while (!process->wait(0));

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
