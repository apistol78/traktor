#ifndef traktor_amalgam_EditorPlugin_H
#define traktor_amalgam_EditorPlugin_H

#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Thread.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPlugin.h"
#include "Ui/Event.h"

namespace traktor
{
	namespace amalgam
	{

class ITargetAction;
class TargetInstance;
class TargetListControl;
class TargetManager;

class EditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	EditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual bool handleCommand(const ui::Command& command, bool result);

	virtual void handleDatabaseEvent(const Guid& eventId);

private:
	editor::IEditor* m_editor;
	Ref< ui::Widget > m_parent;
	Ref< editor::IEditorPageSite > m_site;
	Ref< TargetListControl > m_targetList;

	Ref< TargetManager > m_targetManager;							//!< Target connection manager.
	Ref< db::ConnectionManager > m_connectionManager;		//!< Remote database connection manager.

	RefArray< TargetInstance > m_targetInstances;

	Semaphore m_targetActionQueueLock;
	Signal m_targetActionQueueSignal;
	RefArray< ITargetAction > m_targetActionQueue;

	Thread* m_threadTargetManager;
	Thread* m_threadConnectionManager;
	Thread* m_threadTargetActions;

	void collectTargets();

	void eventTargetPlay(ui::Event* event);

	void eventTargetBuild(ui::Event* event);

	void eventTargetStop(ui::Event* event);

	void threadTargetManager();

	void threadConnectionManager();

	void threadTargetActions();
};

	}
}

#endif	// traktor_amalgam_EditorPlugin_H
