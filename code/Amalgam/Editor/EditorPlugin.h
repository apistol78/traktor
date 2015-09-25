#ifndef traktor_amalgam_EditorPlugin_H
#define traktor_amalgam_EditorPlugin_H

#include <list>
#include <map>
#include "Amalgam/Editor/Tool/ITargetAction.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Thread.h"
#include "Database/Remote/Server/ConnectionManager.h"
#include "Net/Http/HttpServer.h"
#include "Editor/IEditor.h"
#include "Editor/IEditorPlugin.h"
#include "Ui/Event.h"

namespace traktor
{

class IProcess;

	namespace net
	{

class DiscoveryManager;
class HttpServer;

	}

	namespace ui
	{

class TimerEvent;

		namespace custom
		{

class ToolBar;
class ToolBarButtonClickEvent;
class ToolBarDropDown;
class ToolBarDropMenu;

		}
	}

	namespace amalgam
	{

class HostEnumerator;
class Target;
class TargetCaptureEvent;
class TargetListControl;
class TargetInstance;
class TargetManager;
class TargetPlayEvent;
class TargetStopEvent;
class TargetScriptDebuggerSessions;

/*! \brief Amalgam editor plugin.
 * \ingroup Amalgam
 */
class EditorPlugin : public editor::IEditorPlugin
{
	T_RTTI_CLASS;

public:
	EditorPlugin(editor::IEditor* editor);

	virtual bool create(ui::Widget* parent, editor::IEditorPageSite* site);

	virtual void destroy();

	virtual bool handleCommand(const ui::Command& command, bool result);

	virtual void handleDatabaseEvent(db::Database* database, const Guid& eventId);

	virtual void handleWorkspaceOpened();

	virtual void handleWorkspaceClosed();

private:
	struct EditTarget
	{
		Guid guid;
		std::wstring name;
		Ref< const Target > target;
	};

	struct Action
	{
		Ref< ITargetAction::IProgressListener > listener;
		Ref< ITargetAction > action;
	};

	struct ActionChain
	{
		Ref< TargetInstance > targetInstance;
		std::list< Action > actions;
	};

	typedef std::list< ActionChain > action_queue_t;

	editor::IEditor* m_editor;
	Ref< ui::Widget > m_parent;
	Ref< editor::IEditorPageSite > m_site;

	// \name UI
	// \{
	Ref< ui::custom::ToolBar > m_toolBar;
	Ref< ui::custom::ToolBarDropDown > m_toolTargets;
	Ref< ui::custom::ToolBarDropMenu > m_toolTweaks;
	Ref< ui::custom::ToolBarDropDown > m_toolLanguage;
	Ref< TargetListControl > m_targetList;
	// \}

	// \name Tool
	// \{
	Ref< TargetScriptDebuggerSessions > m_targetDebuggerSessions;
	std::vector< EditTarget > m_targets;
	RefArray< TargetInstance > m_targetInstances;
	// \}

	// \name Server
	// \{
	Ref< TargetManager > m_targetManager;					//!< Target connection manager.
	Ref< net::DiscoveryManager > m_discoveryManager;
	Ref< HostEnumerator > m_hostEnumerator;
	Ref< db::ConnectionManager > m_connectionManager;		//!< Remote database connection manager.
	// \}

	// \name Action Worker
	// \{
	Semaphore m_targetActionQueueLock;
	Signal m_targetActionQueueSignal;
	action_queue_t m_targetActionQueue;
	// \}

	// \name HTTP server
	// \{
	Ref< net::HttpServer > m_httpServer;
	// \}

	Thread* m_threadHostEnumerator;
	Thread* m_threadTargetActions;
	Thread* m_threadHttpServer;
	Ref< IProcess > m_pipelineSlaveProcess;

	void updateTargetLists();

	void updateTargetManagers();

	void eventToolBarClick(ui::custom::ToolBarButtonClickEvent* event);

	void eventTargetListPlay(TargetPlayEvent* event);

	void eventTargetListStop(TargetStopEvent* event);

	void eventTargetListShowProfiler(TargetCaptureEvent* event);

	void eventTimer(ui::TimerEvent* event);

	void threadHostEnumerator();

	void threadTargetActions();

	void threadHttpServer();
};

	}
}

#endif	// traktor_amalgam_EditorPlugin_H
