#include "Amalgam/Editor/BuildTargetAction.h"
#include "Amalgam/Editor/PipeReader.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/PlatformInstance.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Editor/IEditor.h"
#include "Xml/XmlDeserializer.h"
#include "Xml/XmlSerializer.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.BuildTargetAction", BuildTargetAction, ITargetAction)

BuildTargetAction::BuildTargetAction(const editor::IEditor* editor, PlatformInstance* platformInstance, TargetInstance* targetInstance)
:	m_editor(editor)
,	m_platformInstance(platformInstance)
,	m_targetInstance(targetInstance)
{
}

bool BuildTargetAction::execute()
{
	const Platform* platform = m_platformInstance->getPlatform();
	T_ASSERT (platform);

	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsBuilding);

	// Read platform default pipeline configuration.
	Ref< Settings > pipelineConfiguration = Settings::read< xml::XmlDeserializer >(FileSystem::getInstance().open(
		platform->getPipelineConfiguration(),
		File::FmRead
	));
	if (!pipelineConfiguration)
		return false;

	// Merge with target pipeline configuration.
	if (!target->getPipelineConfiguration().empty())
	{
		Ref< Settings > targetPipelineConfiguration = Settings::read< xml::XmlDeserializer >(FileSystem::getInstance().open(
			target->getPipelineConfiguration(),
			File::FmRead
		));
		if (!targetPipelineConfiguration)
			return false;

		pipelineConfiguration->merge(targetPipelineConfiguration, true);
	}

	// Set database connection strings.
	db::ConnectionString sourceDatabaseCs = m_editor->getSettings()->getProperty< PropertyString >(L"Editor.SourceDatabase");
	db::ConnectionString outputDatabaseCs(L"provider=traktor.db.LocalDatabase;groupPath=db;binary=true");

	if (sourceDatabaseCs.have(L"groupPath"))
	{
		Path groupPath = FileSystem::getInstance().getAbsolutePath(sourceDatabaseCs.get(L"groupPath"));
		sourceDatabaseCs.set(L"groupPath", groupPath.getPathName());
	}

	pipelineConfiguration->setProperty< PropertyString >(L"Editor.SourceDatabase", sourceDatabaseCs.format());
	pipelineConfiguration->setProperty< PropertyString >(L"Editor.OutputDatabase", outputDatabaseCs.format());

	// Set asset path.
	Path assetPath = m_editor->getSettings()->getProperty< PropertyString >(L"Pipeline.AssetPath");
	assetPath = FileSystem::getInstance().getAbsolutePath(assetPath);
	pipelineConfiguration->setProperty< PropertyString >(L"Pipeline.AssetPath", assetPath.getPathName());

	// Ensure output directory exists.
	std::wstring outputPath = L"output/" + m_targetInstance->getName() + L"/" + m_platformInstance->getName();
	if (!FileSystem::getInstance().makeAllDirectories(outputPath))
	{
		log::error << L"Unable to create output path \"" << outputPath << L"\"" << Endl;
		return false;
	}

	// Write generated pipeline configuration in output directory.
	pipelineConfiguration->write< xml::XmlSerializer >(FileSystem::getInstance().open(
		outputPath + L"/Pipeline.config",
		File::FmWrite
	));

	// Launch pipeline through deploy tool; set cwd to output directory.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
	envmap[L"DEPLOY_PROJECTNAME"] = m_targetInstance->getName();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathNameNoVolume();
#endif

	StringOutputStream ss;
	ss << L"build";

	const Guid& rootAsset = target->getRootAsset();
	if (rootAsset.isValid() && !rootAsset.isNull())
		ss << L" " << rootAsset.format();

	Ref< IProcess > process = OS::getInstance().execute(
		platform->getDeployTool(),
		ss.str(),
		outputPath,
		&envmap,
		true, true, false
	);
	if (!process)
	{
		log::error << L"Failed to launch process \"" << platform->getDeployTool() << L"\"" << Endl;
		m_targetInstance->setState(TsIdle);
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
						m_targetInstance->setBuildProgress((100 * index) / count);
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

	return process->exitCode() == 0;
}

	}
}
