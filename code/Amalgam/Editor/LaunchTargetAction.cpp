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

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.LaunchTargetAction", LaunchTargetAction, ITargetAction)

LaunchTargetAction::LaunchTargetAction(PlatformInstance* platformInstance, TargetInstance* targetInstance)
:	m_platformInstance(platformInstance)
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
	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
	envmap[L"DEPLOY_EXECUTABLE"] = target->getExecutable();

	Ref< IProcess > process = OS::getInstance().execute(
		platform->getDeployTool(),
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
		log::error << L"Failed to launch process \"" << platform->getDeployTool() << L"\"" << Endl;
		m_targetInstance->setState(TsIdle);
		return false;
	}

	return true;
}

	}
}
