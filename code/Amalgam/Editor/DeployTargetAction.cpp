#include "Core/Io/FileSystem.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"
#include "Amalgam/Editor/DeployTargetAction.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.DeployTargetAction", DeployTargetAction, ITargetAction)

DeployTargetAction::DeployTargetAction(TargetInstance* targetInstance)
:	m_targetInstance(targetInstance)
{
}

bool DeployTargetAction::execute()
{
	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsDeploying);

	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();

	std::wstring targetPath = target->getTargetPath();
	std::wstring deployTool = target->getDeployTool();
	std::wstring executable = target->getExecutable();

	OS::envmap_t envmap = OS::getInstance().getEnvironment();

	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
	envmap[L"DEPLOY_TARGETPATH"] = targetPath;
	envmap[L"DEPLOY_DEPLOYTOOL"] = deployTool;
	envmap[L"DEPLOY_EXECUTABLE"] = executable;

	Ref< IProcess > process = OS::getInstance().execute(
		deployTool,
		L"deploy",
		targetPath,
		&envmap,
		true,
		true,
		false
	);
	if (!process)
	{
		m_targetInstance->setState(TsIdle);
		return false;
	}

	if (!process->wait())
	{
		m_targetInstance->setState(TsIdle);
		return false;
	}

	// Deployment finished.

	return process->exitCode() == 0;
}

	}
}
