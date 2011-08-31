#include <list>
#include "Amalgam/Editor/MigrateTargetAction.h"
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
#include "Core/Settings/PropertyStringArray.h"
#include "Core/Settings/Settings.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Database/ConnectionString.h"
#include "Editor/IEditor.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.MigrateTargetAction", MigrateTargetAction, ITargetAction)

MigrateTargetAction::MigrateTargetAction(const editor::IEditor* editor, PlatformInstance* platformInstance, TargetInstance* targetInstance)
:	m_editor(editor)
,	m_platformInstance(platformInstance)
,	m_targetInstance(targetInstance)
{
}

bool MigrateTargetAction::execute()
{
	const Platform* platform = m_platformInstance->getPlatform();
	T_ASSERT (platform);

	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsBuilding);
	m_targetInstance->setBuildProgress(2);

	const Settings* editorSettings = m_editor->getSettings();
	T_ASSERT (editorSettings != 0);

	// Ensure output directory exists.
	std::wstring outputPath = L"output/" + m_targetInstance->getName() + L"/" + m_platformInstance->getName();
	if (!FileSystem::getInstance().makeAllDirectories(outputPath))
	{
		log::error << L"Unable to create output path \"" << outputPath << L"\"" << Endl;
		return false;
	}

	// Set database connection strings.
	db::ConnectionString sourceDatabaseCs(L"provider=traktor.db.LocalDatabase;groupPath=db;binary=true;eventFile=false");
	db::ConnectionString targetDatabaseCs(L"provider=traktor.db.CompactDatabase;fileName=Content.compact");

	// Concate all modules.
	StringOutputStream ms;
	std::vector< std::wstring > modules = editorSettings->getProperty< PropertyStringArray >(L"Editor.Modules");
	for (std::vector< std::wstring >::const_iterator i = modules.begin(); i != modules.end(); ++i)
		ms << *i << L" ";

	// Launch migration through deploy tool; set cwd to output directory.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
	envmap[L"DEPLOY_PROJECTNAME"] = m_targetInstance->getName();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathNameNoVolume();
#endif
	envmap[L"DEPLOY_SOURCE_CS"] = sourceDatabaseCs.format();
	envmap[L"DEPLOY_TARGET_CS"] = targetDatabaseCs.format();
	envmap[L"DEPLOY_MODULES"] = ms.str();

	const DeployTool& deployTool = platform->getDeployTool();
	envmap.insert(deployTool.getEnvironment().begin(), deployTool.getEnvironment().end());

	StringOutputStream ss;
	ss << L"migrate";

	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable(),
		ss.str(),
		outputPath,
		&envmap,
		true, true, false
	);
	if (!process)
	{
		log::error << L"Failed to launch process \"" << deployTool.getExecutable() << L"\"" << Endl;
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
						m_targetInstance->setBuildProgress(2 + (98 * index) / count);
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
