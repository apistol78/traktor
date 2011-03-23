#include "Amalgam/Editor/LaunchTargetAction.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/PlatformInstance.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Io/IStream.h"
#include "Core/Io/StringOutputStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/String.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/Settings.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Editor/IEditor.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.LaunchTargetAction", LaunchTargetAction, ITargetAction)

LaunchTargetAction::LaunchTargetAction(const editor::IEditor* editor, PlatformInstance* platformInstance, TargetInstance* targetInstance)
:	m_editor(editor)
,	m_platformInstance(platformInstance)
,	m_targetInstance(targetInstance)
{
}

bool LaunchTargetAction::execute()
{
	const Platform* platform = m_platformInstance->getPlatform();
	T_ASSERT (platform);

	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsLaunching);

	std::wstring outputPath = L"output/" + m_targetInstance->getName() + L"/" + m_platformInstance->getName();

	// Launch application through deploy tool.
	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();
	OS::envmap_t envmap = OS::getInstance().getEnvironment();
	envmap[L"DEPLOY_PROJECTNAME"] = m_targetInstance->getName();
#if defined(_WIN32)
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
#else
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathNameNoVolume();
#endif
	envmap[L"DEPLOY_EXECUTABLE"] = target->getExecutable();

	// Merge tool environment variables.
	const DeployTool& deployTool = platform->getDeployTool();
	envmap.insert(deployTool.getEnvironment().begin(), deployTool.getEnvironment().end());

	// Merge settings environment variables.
	Ref< PropertyGroup > settingsEnvironment = m_editor->getSettings()->getProperty< PropertyGroup >(L"Amalgam.Environment");
	if (settingsEnvironment)
	{
		const std::map< std::wstring, Ref< IPropertyValue > >& values = settingsEnvironment->getValues();
		for (std::map< std::wstring, Ref< IPropertyValue > >::const_iterator i = values.begin(); i != values.end(); ++i)
		{
			PropertyString* value = dynamic_type_cast< PropertyString* >(i->second);
			if (value)
			{
				envmap.insert(std::make_pair(
					i->first,
					PropertyString::get(value)
				));
			}
		}
	}

	// Launch deploy process.
	Ref< IProcess > process = OS::getInstance().execute(
		deployTool.getExecutable(),
		L"launch",
		outputPath,
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
		m_targetInstance->setState(TsIdle);
		return false;
	}

	return true;
}

	}
}
