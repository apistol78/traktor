#include "Amalgam/Editor/BuildTargetAction.h"
#include "Amalgam/Editor/DeployTargetAction.h"
#include "Amalgam/Editor/EditorPlugin.h"
#include "Amalgam/Editor/LaunchTargetAction.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/PlatformInstance.h"
#include "Amalgam/Editor/PostTargetAction.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetListControl.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/Settings.h"
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
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

const uint16_t c_remoteDatabasePort = 35000;
const uint16_t c_targetConnectionPort = 36000;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.EditorPlugin", EditorPlugin, editor::IEditorPlugin)

EditorPlugin::EditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
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

	// Get defined platforms.
	collectPlatforms();

	// Create target manager.
	m_targetManager = new TargetManager();
	if (m_targetManager->create(
		m_editor->getSettings()->getProperty< PropertyInteger >(L"Amalgam.TargetManagerPort", c_targetConnectionPort)
	))
	{
		collectTargets();

		// Create target manager communication thread.
		m_threadTargetManager = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadTargetManager), L"Target manager");
		m_threadTargetManager->start();
	}
	else
	{
		log::error << L"Unable to create target manager; target manager disabled" << Endl;
		m_targetManager = 0;
	}

	// Create interface.
	Ref< ui::Container > container = new ui::Container();
	container->create(m_parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	container->setText(L"Targets");

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);

	m_toolPlatforms = new ui::custom::ToolBarDropDown(ui::Command(L"Amalgam.View"), 150, i18n::Text(L"AMALGAM_PLATFORMS"));
	for (RefArray< PlatformInstance >::const_iterator i = m_platformInstances.begin(); i != m_platformInstances.end(); ++i)
		m_toolPlatforms->add((*i)->getName());
	m_toolPlatforms->select(0);
	m_toolBar->addItem(m_toolPlatforms);

	m_targetList = new TargetListControl();
	m_targetList->create(container);
	for (RefArray< TargetInstance >::iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
		m_targetList->add(*i);

	m_targetList->addPlayEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetPlay));
	m_targetList->addStopEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetStop));

	m_site->createAdditionalPanel(container, 200, false);

	// Create database server; create configuration from targets.
	Ref< db::Configuration > configuration = new db::Configuration();
	configuration->setListenPort(
		m_editor->getSettings()->getProperty< PropertyInteger >(L"Amalgam.RemoteDatabasePort", c_remoteDatabasePort)
	);

	for (RefArray< TargetInstance >::iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
	{
		std::wstring targetName = (*i)->getName();
		const Target* target = (*i)->getTarget();

		for (RefArray< PlatformInstance >::const_iterator j = m_platformInstances.begin(); j != m_platformInstances.end(); ++j)
		{
			std::wstring platformName = (*j)->getName();
			const Platform* platform = (*j)->getPlatform();

			std::wstring remoteId = targetName + L"|" + platformName;
			std::wstring outputPath = L"output/" + targetName + L"/" + platformName;
			std::wstring databasePath = outputPath + L"/db";
			std::wstring databaseCs = L"provider=traktor.db.LocalDatabase;groupPath=" + databasePath + L";binary=true";

			configuration->setConnectionString(remoteId, databaseCs);
		}
	}

	m_connectionManager = new db::ConnectionManager();
	if (m_connectionManager->create(configuration))
	{
		m_threadConnectionManager = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadConnectionManager), L"Connection manager");
		m_threadConnectionManager->start();
	}
	else
	{
		log::warning << L"Unable to create connection manager; remote database server disabled" << Endl;
		m_connectionManager = 0;
	}

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

	safeDestroy(m_connectionManager);
	safeDestroy(m_targetManager);

	net::Network::finalize();
}

bool EditorPlugin::handleCommand(const ui::Command& command, bool result_)
{
	bool result = true;

	if (command == L"Editor.Build")
	{
		/*
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
				m_targetActionQueue.push_back(new BuildTargetAction(m_editor, targetInstance));
				m_targetActionQueue.push_back(new PostTargetAction(targetInstance, TsIdle));
				m_targetActionQueueSignal.set();
			}
		}

		m_targetList->update();
		*/
	}
	else
		result = false;

	return result;
}

void EditorPlugin::handleDatabaseEvent(const Guid& eventId)
{
}

void EditorPlugin::collectPlatforms()
{
	m_platformInstances.clear();

	Ref< db::Database > sourceDatabase = m_editor->getSourceDatabase();
	if (sourceDatabase)
	{
		RefArray< db::Instance > platformInstances;
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< Platform >()), platformInstances);

		for (RefArray< db::Instance >::iterator i = platformInstances.begin(); i != platformInstances.end(); ++i)
		{
			Ref< const Platform > platform = (*i)->getObject< Platform >();
			T_ASSERT (platform);

			m_platformInstances.push_back(new PlatformInstance((*i)->getName(), platform));
		}
	}
}

void EditorPlugin::collectTargets()
{
	m_targetInstances.resize(0);

	Ref< db::Database > sourceDatabase = m_editor->getSourceDatabase();
	if (sourceDatabase)
	{
		RefArray< db::Instance > targetInstances;
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< Target >()), targetInstances);

		for (RefArray< db::Instance >::iterator i = targetInstances.begin(); i != targetInstances.end(); ++i)
		{
			Ref< const Target > target = (*i)->getObject< Target >();
			T_ASSERT (target);

			Ref< TargetInstance > targetInstance = m_targetManager->createInstance((*i)->getName(), target);
			if (targetInstance)
				m_targetInstances.push_back(targetInstance);
		}
	}
}

void EditorPlugin::eventTargetPlay(ui::Event* event)
{
	int32_t platformIndex = m_toolPlatforms->getSelected();
	if (platformIndex < 0)
		return;

	PlatformInstance* platformInstance = m_platformInstances[platformIndex];
	TargetInstance* targetInstance = checked_type_cast< TargetInstance*, false >(event->getItem());
	Guid activeGuid;

	if (targetInstance->getState() != TsIdle)
		return;

	bool publish = m_editor->getSettings()->getProperty< PropertyBoolean >(L"Amalgam.PublishActiveGuid", true);
	if (publish)
	{
		// Publish active editor's guid to deploying application.
		editor::IEditorPage* activeEditorPage = m_editor->getActiveEditorPage();
		if (activeEditorPage)
		{
			Ref< db::Instance > dataInstance = activeEditorPage->getDataInstance();
			if (dataInstance)
				activeGuid = dataInstance->getGuid();
		}
	}

	targetInstance->setState(TsPending);
	targetInstance->setBuildProgress(0);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

		ActionChain chain;
		chain.actions.push_back(new BuildTargetAction(m_editor, platformInstance, targetInstance));

		if ((event->getKeyState() & ui::KsShift) == 0)
		{
			chain.actions.push_back(new DeployTargetAction(m_editor, platformInstance, targetInstance, activeGuid));
			chain.actions.push_back(new LaunchTargetAction(m_editor, platformInstance, targetInstance));
		}

		chain.postSuccess =
		chain.postFailure = new PostTargetAction(targetInstance, TsIdle);

		m_targetActionQueue.push_back(chain);
		m_targetActionQueueSignal.set();
	}

	m_targetList->update();
}

void EditorPlugin::eventTargetStop(ui::Event* event)
{
	// \fixme
}

void EditorPlugin::threadTargetManager()
{
	while (!m_threadTargetManager->stopped())
		m_targetManager->update();
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
		for (RefArray< ITargetAction >::const_iterator i = chain.actions.begin(); i != chain.actions.end(); ++i)
		{
			success &= (*i)->execute();
			if (!success)
			{
				log::error << L"Deploy action \"" << type_name(*i) << L"\" failed; unable to continue." << Endl;
				break;
			}
		}

		if (success && chain.postSuccess)
			chain.postSuccess->execute();
		else if (!success && chain.postFailure)
			chain.postFailure->execute();
	}
}

	}
}
