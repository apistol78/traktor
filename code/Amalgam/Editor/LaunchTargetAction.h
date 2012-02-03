#ifndef traktor_amalgam_LaunchTargetAction_H
#define traktor_amalgam_LaunchTargetAction_H

#include "Amalgam/Editor/ITargetAction.h"
#include "Core/Ref.h"

namespace traktor
{
	namespace editor
	{

class IEditor;

	}

	namespace amalgam
	{

class PlatformInstance;
class TargetInstance;

class LaunchTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	LaunchTargetAction(
		const editor::IEditor* editor,
		PlatformInstance* platformInstance,
		TargetInstance* targetInstance,
		const std::wstring& deployHost
	);

	virtual bool execute();

private:
	const editor::IEditor* m_editor;
	Ref< PlatformInstance > m_platformInstance;
	Ref< TargetInstance > m_targetInstance;
	std::wstring m_deployHost;
};

	}
}

#endif	// traktor_amalgam_LaunchTargetAction_H