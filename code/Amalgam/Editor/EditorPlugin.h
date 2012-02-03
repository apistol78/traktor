#ifndef traktor_amalgam_EditorPlugin_H
#define traktor_amalgam_EditorPlugin_H

#include <list>
#include <map>
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
	namespace net
	{

class DiscoveryManager;

	}

	namespace ui
	{
		namespace custom
		{

class ToolBar;
class ToolBarDropDown;

		}
	}

	namespace amalgam
	{

class ITargetAction;
class HostEnumerator;
class PlatformInstance;
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
	struct ActionChain
	{
		RefArray< ITargetAction > actions;
		Ref< ITargetAction > postSuccess;
		Ref< ITargetAction > postFailure;
	};

	editor::IEditor* m_editor;
	Ref< ui::Widget > m_parent;
	Ref< editor::IEditorPageSite > m_site;
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarDropDown > m_toolPlatforms;
	Ref< TargetListControl > m_targetList;
	Ref< TargetManager > m_targetManager;					//!< Target connection manager.
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< HostEnumerator > m_hostEnumerator;
	Ref< db::ConnectionManager > m_connectionManager;		//!< Remote database connection manager.
	RefArray< PlatformInstance > m_platformInstances;
	RefArray< TargetInstance > m_targetInstances;
	Semaphore m_targetActionQueueLock;
	Signal m_targetActionQueueSignal;
	std::list< ActionChain > m_targetActionQueue;
	Thread* m_threadTargetManager;
	Thread* m_threadConnectionManager;
	Thread* m_threadTargetActions;

	void collectPlatforms();

	void collectTargets();

	void eventTargetPlay(ui::Event* event);

	void eventTargetStop(ui::Event* event);

	void threadTargetManager();

	void threadConnectionManager();

	void threadTargetActions();
};

	}
}

#endif	// traktor_amalgam_EditorPlugin_H
