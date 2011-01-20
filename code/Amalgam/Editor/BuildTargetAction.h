#ifndef traktor_amalgam_BuildTargetAction_H
#define traktor_amalgam_BuildTargetAction_H

#include "Core/Ref.h"
#include "Amalgam/Editor/ITargetAction.h"

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

class BuildTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	BuildTargetAction(const editor::IEditor* editor, PlatformInstance* platformInstance, TargetInstance* targetInstance);

	virtual bool execute();

private:
	const editor::IEditor* m_editor;
	Ref< PlatformInstance > m_platformInstance;
	Ref< TargetInstance > m_targetInstance;
};

	}
}

#endif	// traktor_amalgam_BuildTargetAction_H