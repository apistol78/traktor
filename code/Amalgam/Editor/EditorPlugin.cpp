#include <cstring>
#include "Amalgam/Editor/EditorPlugin.h"
#include "Amalgam/Editor/HostEnumerator.h"
#include "Amalgam/Editor/Platform.h"
#include "Amalgam/Editor/Target.h"
#include "Amalgam/Editor/TargetConfiguration.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetInstance.h"
#include "Amalgam/Editor/TargetManager.h"
#include "Amalgam/Editor/TargetScriptDebuggerSessions.h"
#include "Amalgam/Editor/Tool/BuildTargetAction.h"
#include "Amalgam/Editor/Tool/DeployTargetAction.h"
#include "Amalgam/Editor/Tool/LaunchTargetAction.h"
#include "Amalgam/Editor/Tool/MigrateTargetAction.h"
#include "Amalgam/Editor/Ui/TargetListControl.h"
#include "Amalgam/Editor/Ui/TargetInstanceListItem.h"
#include "Amalgam/Impl/CaptureScreenShot.h"
#include "Amalgam/Impl/CapturedScreenShot.h"
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Settings/PropertyBoolean.h"
#include "Core/Settings/PropertyFloat.h"
#include "Core/Settings/PropertyGroup.h"
#include "Core/Settings/PropertyInteger.h"
#include "Core/Settings/PropertyString.h"
#include "Core/Settings/PropertyStringSet.h"
#include "Core/Thread/Acquire.h"
#include "Core/Thread/ThreadManager.h"
#include "Database/Database.h"
#include "Database/Group.h"
#include "Database/Instance.h"
#include "Database/Traverse.h"
#include "Drawing/Image.h"
#include "Drawing/PixelFormat.h"
#include "Editor/IEditorPage.h"
#include "Editor/IEditorPageSite.h"
#include "I18N/Text.h"
#include "Net/BidirectionalObjectTransport.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/TableLayout.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarDropDown.h"
#include "Ui/Custom/ToolBar/ToolBarDropMenu.h"
#include "Ui/Custom/ToolBar/ToolBarEmbed.h"
#include "Ui/Custom/ToolBar/ToolBarSeparator.h"
#include "Ui/Events/CommandEvent.h"

namespace traktor
{
	namespace amalgam
	{
		namespace
		{

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
		m_targetListControl->requestUpdate();
	}

private:
	Ref< TargetListControl > m_targetListControl;
	Ref< TargetInstance > m_targetInstance;
	TargetState m_targetState;
};

Ref< ui::MenuItem > createTweakMenuItem(const std::wstring& text, bool initiallyChecked)
{
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(text, true, 0);
	menuItem->setChecked(initiallyChecked);
	return menuItem;
}

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.EditorPlugin", EditorPlugin, editor::IEditorPlugin)

EditorPlugin::EditorPlugin(editor::IEditor* editor)
:	m_editor(editor)
,	m_threadHostEnumerator(0)
,	m_threadTargetActions(0)
{
}

bool EditorPlugin::create(ui::Widget* parent, editor::IEditorPageSite* site)
{
	m_parent = parent;
	m_site = site;

	// Create host enumerator.
	m_discoveryManager = m_editor->getStoreObject< net::DiscoveryManager >(L"DiscoveryManager");
	m_hostEnumerator = new HostEnumerator(m_editor->getSettings(), m_discoveryManager);

	// Create target script debugger dispatcher.
	m_targetDebuggerSessions = new TargetScriptDebuggerSessions();
	m_editor->setStoreObject(L"ScriptDebuggerSessions", m_targetDebuggerSessions);

	// Create panel.
	Ref< ui::Container > container = new ui::Container();
	container->create(m_parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));
	container->setText(L"Targets");

	// Create toolbar; add all targets as drop down items.
	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(container);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventToolBarClick));

	m_toolTargets = new ui::custom::ToolBarDropDown(ui::Command(L"Amalgam.Targets"), 120, i18n::Text(L"AMALGAM_TARGETS"));
	m_toolBar->addItem(m_toolTargets);

	m_toolBar->addItem(new ui::custom::ToolBarSeparator());

	m_toolTweaks = new ui::custom::ToolBarDropMenu(ui::Command(L"Amalgam.Tweaks"), 70, i18n::Text(L"AMALGAM_TWEAKS"), i18n::Text(L"AMALGAM_TWEAKS_TOOLTIP"));
	m_toolTweaks->add(createTweakMenuItem(L"Mute Audio", false));
	m_toolTweaks->add(createTweakMenuItem(L"Audio \"Write Out\"", false));
	m_toolTweaks->add(createTweakMenuItem(L"Force VBlank Off", false));
	m_toolTweaks->add(createTweakMenuItem(L"Physics 2*dT", false));
	m_toolTweaks->add(createTweakMenuItem(L"Supersample *4", false));
	m_toolTweaks->add(createTweakMenuItem(L"Attach Script Debugger", true));
	m_toolTweaks->add(createTweakMenuItem(L"Attach Script Profiler", false));
	m_toolTweaks->add(createTweakMenuItem(L"Profile Rendering", false));
	m_toolTweaks->add(createTweakMenuItem(L"Disable All DLC", false));
	m_toolBar->addItem(m_toolTweaks);

	// Create target configuration list control.
	m_targetList = new TargetListControl();
	m_targetList->create(container);
	m_targetList->addPlayEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetListPlay));
	m_targetList->addStopEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetListStop));
	m_targetList->addCaptureEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTargetListCapture));

	m_site->createAdditionalPanel(container, 200, false);

	// Create target logging target.
	m_targetLog = m_editor->createLogTarget(L"Target");

	// Create threads.
	m_threadHostEnumerator = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadHostEnumerator), L"Host enumerator");
	m_threadHostEnumerator->start();

	m_threadTargetActions = ThreadManager::getInstance().create(makeFunctor(this, &EditorPlugin::threadTargetActions), L"Targets");
	m_threadTargetActions->start();

	container->addTimerEventHandler(ui::createMethodHandler(this, &EditorPlugin::eventTimer));
	container->startTimer(100);

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

	m_targetLog = 0;
	m_hostEnumerator = 0;
	m_toolTargets = 0;
	m_connectionManager = 0;
	m_discoveryManager = 0;

	safeDestroy(m_targetManager);

	if (m_editor)
		m_editor->setStoreObject(L"ScriptDebuggerSessions", 0);
	m_targetDebuggerSessions = 0;

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
	return false;
}

void EditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
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

				Ref< db::Instance > platformInstance = sourceDatabase->getInstance(targetConfiguration->getPlatform());
				T_ASSERT (platformInstance);

				Ref< const Platform > platform = platformInstance->getObject< Platform >();
				T_ASSERT (platform);

				Ref< TargetInstance > targetInstance = new TargetInstance(et.name, et.target, targetConfiguration, platformInstance->getName(), platform);
				T_ASSERT (targetInstance);

				m_targetInstances.push_back(targetInstance);
			}
		}
	}

	// Add to target drop down.
	for (std::vector< EditTarget >::const_iterator i = m_targets.begin(); i != m_targets.end(); ++i)
		m_toolTargets->add(i->name);

	// Create target manager.
	m_targetManager = new TargetManager(m_targetLog, m_targetDebuggerSessions);
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

	// Get connection manager.
	m_connectionManager = m_editor->getStoreObject< db::ConnectionManager >(L"DbConnectionManager");
	if (m_connectionManager)
	{
		// Create configuration from targets.
		for (RefArray< TargetInstance >::iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
		{
			std::wstring remoteId = (*i)->getDatabaseName();
			std::wstring databasePath = (*i)->getOutputPath() + L"/db";
			std::wstring databaseCs = L"provider=traktor.db.LocalDatabase;groupPath=" + databasePath + L";binary=true";
			m_connectionManager->setConnectionString(remoteId, databaseCs);
		}
	}
}

void EditorPlugin::handleWorkspaceClosed()
{
	m_toolTargets->removeAll();

	safeDestroy(m_targetManager);

	m_targets.resize(0);
	m_targetInstances.resize(0);

	m_connectionManager = 0;
}

void EditorPlugin::eventTargetListPlay(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	TargetInstance* targetInstance = checked_type_cast< TargetInstance*, false >(cmdEvent->getItem());

	// Get selected target host.
	std::wstring deployHost = L"";
	int32_t deployHostId = targetInstance->getDeployHostId();
	if (deployHostId >= 0)
		m_hostEnumerator->getHost(deployHostId, deployHost);

	// Resolve absolute output path.
	std::wstring outputPath = FileSystem::getInstance().getAbsolutePath(targetInstance->getOutputPath()).getPathName();

	// Set target's state to pending as actions can be queued up to be performed much later.
	targetInstance->setState(TsPending);
	targetInstance->setBuildProgress(0);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

		ActionChain chain;
		chain.targetInstance = targetInstance;

		Action action;

		// Expose _DEBUG script definition when launching through editor, ie not migrating.
		Ref< PropertyGroup > pipelineSettings = new PropertyGroup();
		if ((event->getKeyState() & ui::KsControl) == 0)
		{
			std::set< std::wstring > scriptPrepDefinitions;
			scriptPrepDefinitions.insert(L"_DEBUG");
			pipelineSettings->setProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions", scriptPrepDefinitions);
		}

		// Add build output data action.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsBuilding);
		action.action = new BuildTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			pipelineSettings,
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			outputPath
		);
		chain.actions.push_back(action);

		if (event->getKeyState() == 0)
		{
			Ref< PropertyGroup > tweakSettings = new PropertyGroup();

			if (m_toolTweaks->get(0)->isChecked())
				tweakSettings->setProperty< PropertyFloat >(L"Audio.MasterVolume", 0.0f);
			if (m_toolTweaks->get(1)->isChecked())
				tweakSettings->setProperty< PropertyBoolean >(L"Audio.WriteOut", true);
			if (m_toolTweaks->get(2)->isChecked())
				tweakSettings->setProperty< PropertyBoolean >(L"Render.WaitVBlank", false);
			if (m_toolTweaks->get(3)->isChecked())
				tweakSettings->setProperty< PropertyFloat >(L"Physics.TimeScale", 2.0f);
			if (m_toolTweaks->get(4)->isChecked())
				tweakSettings->setProperty< PropertyInteger >(L"World.SuperSample", 2);
			if (m_toolTweaks->get(5)->isChecked())
				tweakSettings->setProperty< PropertyBoolean >(L"Script.AttachDebugger", true);
			if (m_toolTweaks->get(6)->isChecked())
				tweakSettings->setProperty< PropertyBoolean >(L"Script.AttachProfiler", true);
			if (m_toolTweaks->get(7)->isChecked())
			{
				std::set< std::wstring > modules = tweakSettings->getProperty< PropertyStringSet >(L"Amalgam.Modules");
				modules.insert(L"Traktor.Render.Capture");
				tweakSettings->setProperty< PropertyStringSet >(L"Amalgam.Modules", modules);
				tweakSettings->setProperty< PropertyString >(L"Render.CaptureType", L"traktor.render.RenderSystemCapture");
			}
			if (m_toolTweaks->get(8)->isChecked())
				tweakSettings->setProperty< PropertyBoolean >(L"Online.DownloadableContent", false);

			// Add deploy and launch actions.
			action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsDeploying);
			action.action = new DeployTargetAction(
				m_editor->getSourceDatabase(),
				m_editor->getSettings(),
				targetInstance->getName(),
				targetInstance->getTarget(),
				targetInstance->getTargetConfiguration(),
				deployHost,
				targetInstance->getDatabaseName(),
				targetInstance->getId(),
				outputPath,
				tweakSettings
			);
			chain.actions.push_back(action);

			action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsLaunching);
			action.action = new LaunchTargetAction(
				m_editor->getSourceDatabase(),
				m_editor->getSettings(),
				targetInstance->getName(),
				targetInstance->getTarget(),
				targetInstance->getTargetConfiguration(),
				deployHost,
				outputPath
			);
			chain.actions.push_back(action);
		}
		else if ((event->getKeyState() & ui::KsControl) != 0)
		{
			// Add migrate actions.
			action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsMigrating);
			action.action = new MigrateTargetAction(
				m_editor->getSourceDatabase(),
				m_editor->getSettings(),
				targetInstance->getName(),
				targetInstance->getTarget(),
				targetInstance->getTargetConfiguration(),
				deployHost,
				outputPath
			);
			chain.actions.push_back(action);
		}

		m_targetActionQueue.push_back(chain);
		m_targetActionQueueSignal.set();
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTargetListStop(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);

	TargetInstance* targetInstance = checked_type_cast< TargetInstance*, false >(cmdEvent->getItem());
	int32_t connectionId = cmdEvent->getCommand().getId();

	RefArray< TargetConnection > connections = targetInstance->getConnections();
	if (connectionId >= 0 && connectionId < int32_t(connections.size()))
	{
		TargetConnection* connection = connections[connectionId];
		T_ASSERT (connection);

		connection->shutdown();
		targetInstance->removeConnection(connection);
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTargetListCapture(ui::Event* event)
{
	ui::CommandEvent* cmdEvent = checked_type_cast< ui::CommandEvent*, false >(event);

	TargetInstance* targetInstance = checked_type_cast< TargetInstance*, false >(cmdEvent->getItem());
	int32_t connectionId = cmdEvent->getCommand().getId();

	RefArray< TargetConnection > connections = targetInstance->getConnections();
	if (connectionId >= 0 && connectionId < int32_t(connections.size()))
	{
		TargetConnection* connection = connections[connectionId];
		T_ASSERT (connection);

		connection->getTransport()->send(new CaptureScreenShot());

		Ref< CapturedScreenShot > capturedScreenShot;
		if (connection->getTransport()->recv< CapturedScreenShot >(1000, capturedScreenShot) == net::BidirectionalObjectTransport::RtSuccess)
		{
			drawing::Image image(
				drawing::PixelFormat::getA8B8G8R8(),
				capturedScreenShot->getWidth(),
				capturedScreenShot->getHeight()
			);

			std::memcpy(
				image.getData(),
				capturedScreenShot->getData().c_ptr(),
				image.getDataSize()
			);

			ui::Application::getInstance()->getClipboard()->setImage(&image);

			log::info << L"Screen capture ready in clipboard" << Endl;
		}
		else
			log::error << L"Timeout while capturing screenshot" << Endl;
	}
}

void EditorPlugin::eventToolBarClick(ui::Event* event)
{
	int32_t selectedTargetIndex = m_toolTargets->getSelected();
	if (selectedTargetIndex < 0 || selectedTargetIndex >= int32_t(m_targetInstances.size()))
		return;

	const EditTarget& et = m_targets[selectedTargetIndex];
	T_ASSERT (et.target);

	m_targetList->removeAll();

	for (RefArray< TargetInstance >::const_iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
	{
		if ((*i)->getTarget() == et.target)
			m_targetList->add(new TargetInstanceListItem(m_hostEnumerator, *i));
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTimer(ui::Event* event)
{
	if (
		m_targetManager &&
		m_targetManager->update()
	)
		m_targetList->requestUpdate();
}

void EditorPlugin::threadHostEnumerator()
{
	while (!m_threadHostEnumerator->stopped())
	{
		m_hostEnumerator->update();

		// Find first local host.
		std::vector< std::wstring > localDeployPlatforms;
		int32_t localDeployHostId = -1;
		for (int32_t i = 0; i < m_hostEnumerator->count(); ++i)
		{
			if (m_hostEnumerator->isLocal(i))
			{
				localDeployHostId = i;
				break;
			}
		}

		// Auto select local remote server for those instances which have none selected yet.
		if (localDeployHostId >= 0)
		{
			bool needUpdate = false;
			for (RefArray< TargetInstance >::const_iterator i = m_targetInstances.begin(); i != m_targetInstances.end(); ++i)
			{
				if ((*i)->getDeployHostId() < 0)
				{
					std::wstring platformName = (*i)->getPlatformName();
					if (m_hostEnumerator->supportPlatform(localDeployHostId, platformName))
					{
						(*i)->setDeployHostId(localDeployHostId);
						needUpdate = true;
					}
				}
			}
			if (needUpdate)
				m_targetList->requestUpdate();
		}

		m_threadHostEnumerator->sleep(1000);
	}
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

		m_targetList->requestUpdate();

		chain.actions.resize(0);
		chain.targetInstance = 0;
	}
}

	}
}
