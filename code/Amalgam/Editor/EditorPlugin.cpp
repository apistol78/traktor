#include "Amalgam/Editor/EditorPlugin.h"
#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Amalgam/Editor/Tool/BuildTargetAction.h"
#include "Amalgam/Editor/Tool/DeployTargetAction.h"
#include "Amalgam/Editor/Tool/LaunchTargetAction.h"
#include "Amalgam/Editor/Tool/MigrateTargetAction.h"
#include "Amalgam/Editor/Ui/TargetListControl.h"
#include "Amalgam/Editor/Ui/TargetInstanceListItem.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Database/Remote/Server/Configuration.h"
#include "Editor/IEditorPage.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Net/Network.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_remoteDatabasePort = 35000;
const uint16_t c_targetConnectionPort = 36000;

class TargetInstanceProgressListener : public RefCountImpl< ITargetAction::IProgressListener >
{
public:
	TargetInstanceProgressListener(TargetListControl* targetListControl, TargetInstance* targetInstance, TargetState targetState)
	:	m_targetListControl(targetListControl)
	,	m_targetInstance(targetInstance)
	,	m_targetState(targetState)
	{
	}

	void notifyTargetActionProgress(int32_t currentStep, int32_t maxStep)
	{
		m_targetInstance->setState(m_targetState);
		m_targetInstance->setBuildProgress((currentStep * 100) / maxStep);
		m_targetListControl->requestLayout();
		m_targetListControl->requestUpdate();
	}

private:
	Ref< TargetListControl > m_targetListControl;
	Ref< TargetInstance > m_targetInstance;
	TargetState m_targetState;
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.EditorPlugin", EditorPlugin, editor::IEditorPlugin)

EditorPlugin::EditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
,	m_threadHostEnumerator(0)
,	m_threadTargetManager(0)
,	m_threadConnectionManager(0)
,	m_threadTargetActions(0)
{
}

bool EditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	net::Network::initialize();

	m_parent = parent;
	m_site = site;

	// Create host enumerator.
	m_discoveryManager = new net::DiscoveryManager();
	if (!m_discoveryManager->create(false))
		log::error << L"Unable to create discovery manager; unable to enumerate hosts" << Endl;

	m_hostEnumerator = new HostEnumerator(m_discoveryManager);

	// Create panel.
	Ref< ui::Container > container = new ui::Container();
	container->create(m_parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	container->setText(L"Targets");

	// Create toolbar; add all targets as drop down items.
	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventToolBarClick));

	m_toolTargets = new ui::custom::ToolBarDropDown(ui::Command(L"Amalgam.Targets"), 150, i18n::Text(L"AMALGAM_TARGETS"));
	m_toolBar->addItem(m_toolTargets);

	// Create target configuration list control.
	m_targetList = new TargetListControl();
	m_targetList->create(container);
	m_targetList->addPlayEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetListPlay));
	m_targetList->addStopEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetListStop));

	m_site->createAdditionalPanel(container, 200, false);

	// Create threads.
	m_threadHostEnumerator = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadHostEnumerator), L"Host enumerator");
	m_threadHostEnumerator->start();

	m_threadTargetActions = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadTargetActions), L"Targets");
	m_threadTargetActions->start();

	return true;
}

void EditorPlugin::destroy()
{
	if (m_threadTargetActions)
	{
		m_threadTargetActions->stop();
		ThreadManager::getInstance().destroy(m_threadTargetActions);
	}

	if (m_threadHostEnumerator)
	{
		m_threadHostEnumerator->stop();
		ThreadManager::getInstance().destroy(m_threadHostEnumerator);
	}

	m_hostEnumerator = 0;
	m_toolTargets = 0;

	safeDestroy(m_connectionManager);
	safeDestroy(m_discoveryManager);
	safeDestroy(m_targetManager);

	m_targetInstances.clear();
	m_targets.clear();

	safeDestroy(m_targetList);
	safeDestroy(m_toolBar);

	m_site = 0;
	m_parent = 0;
	m_editor = 0;

	net::Network::finalize();
}

bool EditorPlugin::handleCommand(const ui::Command& command, bool result_)
{
	bool result = true;

	if (command == L"Editor.Build")
	{
		// Refresh all running targets.
		for (RefArray< TargetInstance >::iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
		{
			TargetInstance* targetInstance = *i;

			if (targetInstance->getState() != TsIdle)
				continue;
			if (targetInstance->getConnections().empty())
				continue;

			targetInstance->setState(TsPending);
			targetInstance->setBuildProgress(0);

			{
				T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

				ActionChain chain;
				chain.targetInstance = targetInstance;

				Action action;

				// Add build output data action.
				action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsProgress);
				action.action = new BuildTargetAction(
					m_editor->getSourceDatabase(),
					m_editor->getSettings(),
					targetInstance->getTarget(),
					targetInstance->getTargetConfiguration(),
					targetInstance->getOutputPath()
				);
				chain.actions.push_back(action);

				m_targetActionQueue.push_back(chain);
				m_targetActionQueueSignal.set();
			}
		}

		m_targetList->update();
	}
	else
		result = false;

	return result;
}

void EditorPlugin::handleDatabaseEvent(const Guid& eventId)
{
}

void EditorPlugin::handleWorkspaceOpened()
{
	// Get targets from source database.
	Ref< db::Database > sourceDatabase = m_editor->getSourceDatabase();
	if (sourceDatabase)
	{
		RefArray< db::Instance > targetInstances;
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< Target >()), targetInstances);

		for (RefArray< db::Instance >::iterator i = targetInstances.begin(); i != targetInstances.end(); ++i)
		{
			EditTarget et;

			et.name = (*i)->getName();
			et.target = (*i)->getObject< Target >();
			T_ASSERT (et.target);

			m_targets.push_back(et);

			const RefArray< TargetConfiguration >& targetConfigurations = et.target->getConfigurations();
			for (RefArray< TargetConfiguration >::const_iterator j = targetConfigurations.begin(); j != targetConfigurations.end(); ++j)
			{
				TargetConfiguration* targetConfiguration = *j;
				T_ASSERT (targetConfiguration);

				Ref< const Platform > platform = sourceDatabase->getObjectReadOnly< Platform >(targetConfiguration->getPlatform());
				T_ASSERT (platform);

				Ref< TargetInstance > targetInstance = new TargetInstance(et.name, et.target, targetConfiguration, platform);
				T_ASSERT (targetInstance);

				m_targetInstances.push_back(targetInstance);
			}
		}
	}

	// Add to target drop down.
	for (std::vector< EditTarget >::const_iterator i = m_targets.begin(); i != m_targets.end(); ++i)
		m_toolTargets->add(i->name);

	// Create target manager.
	m_targetManager = new TargetManager();
	if (m_targetManager->create(
		m_editor->getSettings()->getProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", c_targetConnectionPort)
	))
	{
		for (RefArray< TargetInstance >::iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
			m_targetManager->addInstance(*i);
	}
	else
	{
		log::error << L"Unable to create target manager; target manager disabled" << Endl;
		m_targetManager = 0;
	}

	// Create database server; create configuration from targets.
	Ref< db::Configuration > configuration = new db::Configuration();
	configuration->setListenPort(
		m_editor->getSettings()->getProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", c_remoteDatabasePort)
	);

	for (RefArray< TargetInstance >::iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
	{
		std::wstring remoteId = (*i)->getDatabaseName();
		std::wstring databasePath = (*i)->getOutputPath() + L"/db";
		std::wstring databaseCs = L"provider=traktor.db.LocalDatabase;groupPath=" + databasePath + L";binary=true";
		configuration->setConnectionString(remoteId, databaseCs);
	}

	m_connectionManager = new db::ConnectionManager();
	if (!m_connectionManager->create(configuration))
	{
		log::warning << L"Unable to create connection manager; remote database server disabled" << Endl;
		m_connectionManager = 0;
	}

	// Create communication threads.
	if (m_targetManager)
	{
		m_threadTargetManager = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadTargetManager), L"Target manager");
		m_threadTargetManager->start();
	}

	if (m_connectionManager)
	{
		m_threadConnectionManager = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadConnectionManager), L"Connection manager");
		m_threadConnectionManager->start();
	}
}

void EditorPlugin::handleWorkspaceClosed()
{
	// Terminate communication threads.
	if (m_threadConnectionManager)
	{
		m_threadConnectionManager->stop();
		ThreadManager::getInstance().destroy(m_threadConnectionManager);
	}

	if (m_threadTargetManager)
	{
		m_threadTargetManager->stop();
		ThreadManager::getInstance().destroy(m_threadTargetManager);
	}

	m_toolTargets->removeAll();

	safeDestroy(m_connectionManager);
	safeDestroy(m_targetManager);

	m_targets.resize(0);
	m_targetInstances.resize(0);
}

void EditorPlugin::eventTargetListPlay(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	TargetInstance* targetInstance = checked_type_cast< TargetInstance*, false >(cmdEvent->getItem());

	std::wstring deployHost = L"";
	m_hostEnumerator->getHost(targetInstance->getDeployHostId(), deployHost);

	// Set target's state to pending as actions can be queued up to be performed much later.
	targetInstance->setState(TsPending);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

		ActionChain chain;
		chain.targetInstance = targetInstance;

		Action action;

		// Add build output data action.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsProgress);
		action.action = new BuildTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			targetInstance->getOutputPath()
		);
		chain.actions.push_back(action);

		if (event->getKeyState() == 0)
		{
			// Add deploy and launch actions.
			action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsProgress);
			action.action = new DeployTargetAction(
				m_editor->getSourceDatabase(),
				m_editor->getSettings(),
				targetInstance->getName(),
				targetInstance->getTarget(),
				targetInstance->getTargetConfiguration(),
				deployHost,
				targetInstance->getDatabaseName(),
				targetInstance->getId(),
				targetInstance->getOutputPath()
			);
			chain.actions.push_back(action);

			action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsPending);
			action.action = new LaunchTargetAction(
				m_editor->getSourceDatabase(),
				m_editor->getSettings(),
				targetInstance->getName(),
				targetInstance->getTarget(),
				targetInstance->getTargetConfiguration(),
				deployHost,
				targetInstance->getOutputPath()
			);
			chain.actions.push_back(action);
		}
		else if ((event->getKeyState() & ui::KsControl) != 0)
		{
			// Add migrate actions.
			action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsProgress);
			action.action = new MigrateTargetAction(
				m_editor->getSourceDatabase(),
				m_editor->getSettings(),
				targetInstance->getName(),
				targetInstance->getTarget(),
				targetInstance->getTargetConfiguration(),
				deployHost,
				targetInstance->getOutputPath()
			);
			chain.actions.push_back(action);
		}

		m_targetActionQueue.push_back(chain);
		m_targetActionQueueSignal.set();
	}

	m_targetList->requestLayout();
	m_targetList->requestUpdate();
}

void EditorPlugin::eventTargetListStop(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	
	TargetInstance* targetInstance = checked_type_cast< TargetInstance*, false >(cmdEvent->getItem());
	int32_t connectionId = cmdEvent->getCommand().getId();

	const RefArray< TargetConnection >& connections = targetInstance->getConnections();
	if (connectionId >= 0 && connectionId < int32_t(connections.size()))
	{
		TargetConnection* connection = connections[connectionId];
		T_ASSERT (connection);

		connection->shutdown();
		targetInstance->removeConnection(connection);
	}

	m_targetList->requestLayout();
	m_targetList->requestUpdate();
}

void EditorPlugin::eventToolBarClick(ui::Event* event)
{
	int32_t selectedTargetIndex = m_toolTargets->getSelected();
	if (selectedTargetIndex < 0 || selectedTargetIndex > m_targetInstances.size())
		return;

	const EditTarget& et = m_targets[selectedTargetIndex];
	T_ASSERT (et.target);

	m_targetList->removeAll();

	for (RefArray< TargetInstance >::const_iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
	{
		if ((*i)->getTarget() == et.target)
			m_targetList->add(new TargetInstanceListItem(m_hostEnumerator, *i));
	}

	m_targetList->requestLayout();
	m_targetList->requestUpdate();
}

void EditorPlugin::threadHostEnumerator()
{
	while (!m_threadHostEnumerator->stopped())
	{
		m_hostEnumerator->update();
		m_threadHostEnumerator->sleep(1000);
	}
}

void EditorPlugin::threadTargetManager()
{
	while (!m_threadTargetManager->stopped())
	{
		if (m_targetManager->update())
		{
			m_targetList->requestLayout();
			m_targetList->requestUpdate();
		}
	}
}

void EditorPlugin::threadConnectionManager()
{
	while (!m_threadConnectionManager->stopped())
		m_connectionManager->update(100);
}

void EditorPlugin::threadTargetActions()
{
	ActionChain chain;
	while (!m_threadTargetActions->stopped())
	{
		if (!m_targetActionQueueSignal.wait(100))
			continue;

		{
			T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);
			if (m_targetActionQueue.empty())
				continue;

			chain = m_targetActionQueue.front();
			m_targetActionQueue.pop_front();

			if (m_targetActionQueue.empty())
				m_targetActionQueueSignal.reset();
		}

		bool success = true;
		for (std::list< Action >::const_iterator i = chain.actions.begin(); i != chain.actions.end(); ++i)
		{
			success &= i->action->execute(i->listener);
			if (!success)
			{
				log::error << L"Deploy action \"" << type_name(i->action) << L"\" failed; unable to continue." << Endl;
				break;
			}
		}

		if (chain.targetInstance)
			chain.targetInstance->setState(TsIdle);

		m_targetList->requestLayout();
		m_targetList->requestUpdate();

		chain.actions.resize(0);
		chain.targetInstance = 0;
	}
}

	}
}
