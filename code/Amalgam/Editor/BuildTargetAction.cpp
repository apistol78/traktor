#include "Amalgam/Editor/BuildTargetAction.h"
#include "Amalgam/Editor/PipeReader.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/Split.h"
#include "Core/Misc/String.h"
#include "Core/System/IProcess.h"
#include "Core/System/OS.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.BuildTargetAction", BuildTargetAction, ITargetAction)

BuildTargetAction::BuildTargetAction(TargetInstance* targetInstance)
:	m_targetInstance(targetInstance)
{
}

bool BuildTargetAction::execute()
{
	const Target* target = m_targetInstance->getTarget();
	T_ASSERT (target);

	m_targetInstance->setState(TsBuilding);

	Path projectRoot = FileSystem::getInstance().getCurrentVolume()->getCurrentDirectory();

	std::wstring targetPath = target->getTargetPath();
	std::wstring deployTool = target->getDeployTool();
	std::wstring executable = target->getExecutable();

	OS::envmap_t envmap = OS::getInstance().getEnvironment();

	envmap[L"DEPLOY_PROJECTROOT"] = projectRoot.getPathName();
	envmap[L"DEPLOY_TARGETPATH"] = targetPath;
	envmap[L"DEPLOY_DEPLOYTOOL"] = deployTool;
	envmap[L"DEPLOY_EXECUTABLE"] = executable;

	StringOutputStream ss;
	ss << L"build";

	const Guid& rootAsset = target->getRootAsset();
	if (rootAsset.isValid() && !rootAsset.isNull())
		ss << L" " << rootAsset.format();

	Ref< IProcess > process = OS::getInstance().execute(
		deployTool,
		ss.str(),
		targetPath,
		&envmap,
		true,
		true,
		false
	);
	if (!process)
	{
		log::error << L"Failed to launch process \"" << deployTool << L"\"" << Endl;
		m_targetInstance->setState(TsIdle);
		return false;
	}

	PipeReader stdOutReader(
		process->getPipeStream(IProcess::SpStdOut)
	);
	PipeReader stdErrReader(
		process->getPipeStream(IProcess::SpStdErr)
	);

	std::wstring str;
	while (!process->wait(100))
	{
		while (stdOutReader.readLine(str, 0))
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
	}

	return process->exitCode() == 0;
}

	}
}
