#ifndef traktor_amalgam_MigrateTargetAction_H
#define traktor_amalgam_MigrateTargetAction_H

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

class MigrateTargetAction : public ITargetAction
{
	T_RTTI_CLASS;

public:
	MigrateTargetAction(const editor::IEditor* editor, PlatformInstance* platformInstance, TargetInstance* targetInstance);

	virtual bool execute();

private:
	const editor::IEditor* m_editor;
	Ref< PlatformInstance > m_platformInstance;
	Ref< TargetInstance > m_targetInstance;
};

	}
}

#endif	// traktor_amalgam_MigrateTargetAction_H