#ifndef traktor_amalgam_DeployTargetAction_H
#define traktor_amalgam_DeployTargetAction_H

#include "Amalgam/Editor/ITargetAction.h"
#include "Core/Guid.h"
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

class DeployTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	DeployTargetAction(
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

#endif	// traktor_amalgam_DeployTargetAction_H