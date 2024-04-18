/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <cstring>
#include "Core/Io/FileSystem.h"
#include "Core/Log/Log.h"
#include "Core/Misc/ObjectStore.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Misc/String.h"
#include "Core/Serialization/DeepClone.h"
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
#include "Net/Network.h"
#include "Net/SocketAddressIPv4.h"
#include "Net/Discovery/DiscoveryManager.h"
#include "Runtime/Editor/EditorPlugin.h"
#include "Runtime/Editor/HostEnumerator.h"
#include "Runtime/Editor/ProfilerDialog.h"
#include "Runtime/Editor/TargetConnection.h"
#include "Runtime/Editor/TargetInstance.h"
#include "Runtime/Editor/TargetManager.h"
#include "Runtime/Editor/Deploy/BuildTargetAction.h"
#include "Runtime/Editor/Deploy/DeployTargetAction.h"
#include "Runtime/Editor/Deploy/LaunchTargetAction.h"
#include "Runtime/Editor/Deploy/MigrateTargetAction.h"
#include "Runtime/Editor/Deploy/Platform.h"
#include "Runtime/Editor/Deploy/Target.h"
#include "Runtime/Editor/Deploy/TargetConfiguration.h"
#include "Runtime/Editor/Ui/TargetBuildEvent.h"
#include "Runtime/Editor/Ui/TargetCaptureEvent.h"
#include "Runtime/Editor/Ui/TargetCommandEvent.h"
#include "Runtime/Editor/Ui/TargetInstanceListItem.h"
#include "Runtime/Editor/Ui/TargetListControl.h"
#include "Runtime/Editor/Ui/TargetMigrateEvent.h"
#include "Runtime/Editor/Ui/TargetPlayEvent.h"
#include "Runtime/Editor/Ui/TargetStopEvent.h"
#include "Runtime/Target/CommandEvent.h"
#include "Script/Editor/ScriptDebuggerSessions.h"
#include "Ui/Application.h"
#include "Ui/CheckBox.h"
#include "Ui/Clipboard.h"
#include "Ui/Command.h"
#include "Ui/Container.h"
#include "Ui/MenuItem.h"
#include "Ui/TableLayout.h"
#include "Ui/ToolBar/ToolBar.h"
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"
#include "Ui/ToolBar/ToolBarDropDown.h"
#include "Ui/ToolBar/ToolBarDropMenu.h"
#include "Ui/ToolBar/ToolBarEmbed.h"
#include "Ui/ToolBar/ToolBarSeparator.h"

namespace traktor::runtime
{
	namespace
	{

const struct { const wchar_t* human; const wchar_t* code; } c_languageCodes[] =
{
	{ L"RUNTIME_LANGUAGE_ENGLISH", L"en" },
	{ L"RUNTIME_LANGUAGE_FRENCH", L"fr" },
	{ L"RUNTIME_LANGUAGE_GERMAN", L"de" },
	{ L"RUNTIME_LANGUAGE_DUTCH", L"nl" },
	{ L"RUNTIME_LANGUAGE_ITALIAN", L"it" },
	{ L"RUNTIME_LANGUAGE_JAPANESE", L"jp" },
	{ L"RUNTIME_LANGUAGE_KOREANA", L"kr" },
	{ L"RUNTIME_LANGUAGE_SPANISH", L"es" },
	{ L"RUNTIME_LANGUAGE_RUSSIAN", L"ru" },
	{ L"RUNTIME_LANGUAGE_POLISH", L"pl" },
	{ L"RUNTIME_LANGUAGE_PORTUGUESE", L"pt" },
	{ L"RUNTIME_LANGUAGE_CHINESE", L"ch" },
	{ L"RUNTIME_LANGUAGE_SWEDISH", L"sv" },
	{ L"RUNTIME_LANGUAGE_ROMANIAN", L"ro" },
	{ L"RUNTIME_LANGUAGE_CZECH", L"cs" }
};

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
	Ref< ui::MenuItem > menuItem = new ui::MenuItem(text, true, nullptr);
	menuItem->setChecked(initiallyChecked);
	return menuItem;
}

	}

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.EditorPlugin", EditorPlugin, editor::IEditorPlugin)

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
	m_discoveryManager = m_editor->getObjectStore()->get< net::DiscoveryManager >();
	m_hostEnumerator = new HostEnumerator(m_editor->getSettings(), m_discoveryManager);

	// Create panel.
	m_container = new ui::Container();
	m_container->create(m_parent, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0_ut, 0_ut));
	m_container->setText(i18n::Text(L"RUNTIME_TARGETS_TITLE"));

	// Create toolbar; add all targets as drop down items.
	m_toolBar = new ui::ToolBar();
	m_toolBar->create(m_container);
	m_toolBar->setEnable(false);
	m_toolBar->addEventHandler< ui::ToolBarButtonClickEvent >(this, &EditorPlugin::eventToolBarClick);

	m_toolTargets = new ui::ToolBarDropDown(ui::Command(L"Runtime.Targets"), 120_ut, i18n::Text(L"RUNTIME_TARGETS"));
	m_toolBar->addItem(m_toolTargets);

	m_toolBar->addItem(new ui::ToolBarSeparator());

	m_toolTweaks = new ui::ToolBarDropMenu(70_ut, i18n::Text(L"RUNTIME_TWEAKS"), true, i18n::Text(L"RUNTIME_TWEAKS_TOOLTIP"));
	m_toolTweaks->add(createTweakMenuItem(L"Mute Audio", false));
	m_toolTweaks->add(createTweakMenuItem(L"Audio \"Write Out\"", false));
	m_toolTweaks->add(createTweakMenuItem(L"Force Render Thread Off", false));
	m_toolTweaks->add(createTweakMenuItem(L"Force VBlank Off", false));
	m_toolTweaks->add(createTweakMenuItem(L"Physics 2*dT", false));
	m_toolTweaks->add(createTweakMenuItem(L"Attach Script Debugger", true));
	m_toolTweaks->add(createTweakMenuItem(L"Attach Script Profiler", false));
	m_toolTweaks->add(createTweakMenuItem(L"Disable All DLC", false));
	m_toolTweaks->add(createTweakMenuItem(L"Disable Adaptive Updates", false));
	m_toolTweaks->add(createTweakMenuItem(L"Launch With 1/4 Window", false));
	m_toolTweaks->add(createTweakMenuItem(L"Disable Baked Lighting", false));
	m_toolTweaks->add(createTweakMenuItem(L"Validate Rendering", false));
	m_toolTweaks->add(createTweakMenuItem(L"Debug Layer", true));
	m_toolBar->addItem(m_toolTweaks);

	m_toolLanguage = new ui::ToolBarDropDown(ui::Command(L"Runtime.Language"), 85_ut, i18n::Text(L"RUNTIME_LANGUAGE"));
	m_toolLanguage->add(i18n::Text(L"RUNTIME_LANGUAGE_DEFAULT"));
	for (uint32_t i = 0; i < sizeof_array(c_languageCodes); ++i)
		m_toolLanguage->add(i18n::Text(c_languageCodes[i].human));
	m_toolLanguage->select(0);
	m_toolBar->addItem(m_toolLanguage);

	// Create target configuration list control.
	m_targetList = new TargetListControl();
	m_targetList->create(m_container);
	m_targetList->addEventHandler< TargetBuildEvent >(this, &EditorPlugin::eventTargetListBuild);
	m_targetList->addEventHandler< TargetCaptureEvent >(this, &EditorPlugin::eventTargetListShowProfiler);
	m_targetList->addEventHandler< TargetMigrateEvent >(this, &EditorPlugin::eventTargetListMigrate);
	m_targetList->addEventHandler< TargetPlayEvent >(this, &EditorPlugin::eventTargetListPlay);
	m_targetList->addEventHandler< TargetStopEvent >(this, &EditorPlugin::eventTargetListStop);
	m_targetList->addEventHandler< TargetCommandEvent >(this, &EditorPlugin::eventTargetListCommand);

	m_site->createAdditionalPanel(m_container, 200_ut, false);

	// Create threads.
	m_threadHostEnumerator = ThreadManager::getInstance().create([this](){ threadHostEnumerator(); }, L"Host enumerator");
	m_threadHostEnumerator->start();

	m_threadTargetActions = ThreadManager::getInstance().create([this](){ threadTargetActions(); }, L"Targets");
	m_threadTargetActions->start();

	m_container->addEventHandler< ui::TimerEvent >(this, &EditorPlugin::eventTimer);
	m_container->startTimer(30);

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

	m_hostEnumerator = nullptr;
	m_toolTargets = nullptr;
	m_connectionManager = nullptr;
	m_discoveryManager = nullptr;

	safeDestroy(m_targetManager);

	m_lastLaunchedTargetInstance = nullptr;
	m_targetInstances.clear();
	m_targets.clear();

	if (m_container)
		m_site->destroyAdditionalPanel(m_container);

	safeDestroy(m_targetList);
	safeDestroy(m_toolBar);
	safeDestroy(m_container);

	m_site = nullptr;
	m_parent = nullptr;
	m_editor = nullptr;
}

bool EditorPlugin::handleCommand(const ui::Command& command, bool result_)
{
	if (command == L"Editor.AutoBuild")
	{
		// Check if we should auto-build for running targets.
		const bool autoBuild = m_editor->getSettings()->getProperty< bool >(L"Runtime.AutoBuildRunningTargets", true);
		if (!autoBuild)
			return false;

		for (auto targetInstance : m_targetInstances)
		{
			// Only build for those who have any running applications.
			if (targetInstance->getConnections().empty())
				continue;

			// Resolve absolute output path.
			const std::wstring outputPath = FileSystem::getInstance().getAbsolutePath(targetInstance->getOutputPath()).getPathName();

			// Set target's state to pending as actions can be queued up to be performed much later.
			targetInstance->setState(TsPending);
			targetInstance->setBuildProgress(0);

			{
				T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

				ActionChain chain;
				chain.targetInstance = targetInstance;

				Action action;

				// Create pipeline settings.
				Ref< PropertyGroup > pipelineSettings = new PropertyGroup();
				SmallSet< std::wstring > scriptPrepDefinitions;
				scriptPrepDefinitions.insert(L"_DEBUG");
				pipelineSettings->setProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions", scriptPrepDefinitions);
				pipelineSettings->setProperty< PropertyBoolean >(L"Pipeline.EditorDeploy", true);

				// Create "tweak" settings.
				Ref< PropertyGroup > tweakSettings = getTweakSettings();

				// Add build output data action.
				action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsBuilding);
				action.action = new BuildTargetAction(
					m_editor->getSourceDatabase(),
					m_editor->getSettings(),
					pipelineSettings,
					targetInstance->getTarget(),
					targetInstance->getTargetConfiguration(),
					outputPath,
					tweakSettings,
					false
				);
				chain.actions.push_back(action);

				m_targetActionQueue.push_back(chain);
				m_targetActionQueueSignal.set();
			}
		}

		m_targetList->requestUpdate();
	}
	else if (command == L"Runtime.Editor.LaunchLast")
	{
		if (m_lastLaunchedTargetInstance)
			launch(m_lastLaunchedTargetInstance);
	}
	else
		return false;

	return true;
}

void EditorPlugin::handleDatabaseEvent(db::Database* database, const Guid& eventId)
{
	for (const auto& target : m_targets)
	{
		if (target.guid == eventId)
		{
			updateTargetLists();
			updateTargetManagers();
			break;
		}
	}
}

void EditorPlugin::handleWorkspaceOpened()
{
	updateTargetLists();

	// Create target manager.
	m_targetManager = new TargetManager(m_editor);
	if (!m_targetManager->create())
	{
		log::error << L"Unable to create target manager; target manager disabled" << Endl;
		m_targetManager = nullptr;
	}

	// Get connection manager.
	m_connectionManager = m_editor->getObjectStore()->get< db::ConnectionManager >();

	// Enable widgets.
	m_toolBar->setEnable(true);

	updateTargetManagers();
}

void EditorPlugin::handleWorkspaceClosed()
{
	m_toolTargets->removeAll();
	m_toolBar->update();

	m_targetList->removeAll();
	m_targetList->requestUpdate();

	safeDestroy(m_targetManager);

	m_targets.resize(0);
	m_targetInstances.resize(0);
	m_lastLaunchedTargetInstance = nullptr;

	m_connectionManager = nullptr;
}

void EditorPlugin::handleEditorClosed()
{
}

void EditorPlugin::updateTargetLists()
{
	m_targets.clear();
	m_targetInstances.clear();
	m_lastLaunchedTargetInstance = nullptr;

	// Get targets from source database.
	Ref< db::Database > sourceDatabase = m_editor->getSourceDatabase();
	if (sourceDatabase)
	{
		RefArray< db::Instance > targetInstances;
		db::recursiveFindChildInstances(sourceDatabase->getRootGroup(), db::FindInstanceByType(type_of< Target >()), targetInstances);

		for (auto targetInstance : targetInstances)
		{
			EditTarget et;

			et.guid = targetInstance->getGuid();
			et.name = targetInstance->getName();
			et.target = targetInstance->getObject< Target >();
			
			if (!et.target)
			{
				log::error << L"Unable to register target instance \"" << targetInstance->getName() << L"\"; unable to read target." << Endl;
				continue;
			}

			m_targets.push_back(et);

			for (auto targetConfiguration : et.target->getConfigurations())
			{
				Ref< db::Instance > platformInstance = sourceDatabase->getInstance(targetConfiguration->getPlatform());
				if (!platformInstance)
				{
					log::error << L"Unable to register target \"" << targetConfiguration->getName() << L"\"; no platform instance found." << Endl;
					continue;
				}

				Ref< const Platform > platform = platformInstance->getObject< Platform >();
				if (!platform)
				{
					log::error << L"Unable to register target \"" << targetConfiguration->getName() << L"\"; unable to read platform instance." << Endl;
					continue;
				}

				m_targetInstances.push_back(new TargetInstance(
					et.name,
					et.target,
					targetConfiguration,
					platformInstance->getName(),
					platform
				));
			}
		}
	}

	m_toolTargets->removeAll();
	m_targetList->removeAll();

	// Add to target drop down.
	if (!m_targets.empty())
	{
		for (const auto& target : m_targets)
			m_toolTargets->add(target.name);

		m_toolTargets->select(0);

		for (auto targetInstance : m_targetInstances)
		{
			if (targetInstance->getTarget() == m_targets[0].target)
				m_targetList->add(new TargetInstanceListItem(m_hostEnumerator, targetInstance));
		}
	}

	m_toolBar->update();
	m_targetList->requestUpdate();
}

void EditorPlugin::updateTargetManagers()
{
	if (m_targetManager)
	{
		m_targetManager->removeAllInstances();
		for (auto targetInstance : m_targetInstances)
			m_targetManager->addInstance(targetInstance);
	}

	if (m_connectionManager)
	{
		for (auto targetInstance : m_targetInstances)
		{
			const std::wstring remoteId = targetInstance->getDatabaseName();
			const std::wstring databasePath = targetInstance->getOutputPath() + L"/db";
			const std::wstring databaseCs = L"provider=traktor.db.LocalDatabase;groupPath=" + databasePath + L";binary=true";
			m_connectionManager->setConnectionString(remoteId, databaseCs);
		}
	}
}

Ref< PropertyGroup > EditorPlugin::getTweakSettings() const
{
	Ref< PropertyGroup > tweakSettings = new PropertyGroup();
	if (m_toolTweaks->get(0)->isChecked())
		tweakSettings->setProperty< PropertyFloat >(L"Audio.MasterVolume", 0.0f);
	if (m_toolTweaks->get(1)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"Audio.WriteOut", true);
	if (m_toolTweaks->get(2)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"Runtime.RenderThread", false);
	if (m_toolTweaks->get(3)->isChecked())
		tweakSettings->setProperty< PropertyInteger >(L"Render.WaitVBlanks", 0);
	if (m_toolTweaks->get(4)->isChecked())
		tweakSettings->setProperty< PropertyFloat >(L"Physics.TimeScale", 0.25f);
	if (m_toolTweaks->get(5)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"Script.AttachDebugger", true);
	if (m_toolTweaks->get(6)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"Script.AttachProfiler", true);
	if (m_toolTweaks->get(7)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"Online.DownloadableContent", false);
	if (m_toolTweaks->get(8)->isChecked())
		tweakSettings->setProperty< PropertyInteger >(L"Runtime.MaxSimulationUpdates", 1);
	if (m_toolTweaks->get(9)->isChecked())
		tweakSettings->setProperty< PropertyInteger >(L"Render.DisplayMode.Window/DefaultDenominator", 4);
	if (m_toolTweaks->get(10)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"BakePipelineOperator.Enable", false);
	if (m_toolTweaks->get(11)->isChecked())
	{
		// Use low level render API validation.
		tweakSettings->setProperty< PropertyBoolean >(L"Render.Validation", true);

		// Use our own high level verification layer also.
		auto modules = tweakSettings->getProperty< SmallSet< std::wstring > >(L"Runtime.Modules");
		modules.insert(L"Traktor.Render.Vrfy");
		tweakSettings->setProperty< PropertyStringSet >(L"Runtime.Modules", modules);
		tweakSettings->setProperty< PropertyString >(L"Render.CaptureType", L"traktor.render.RenderSystemVrfy");
	}
	if (m_toolTweaks->get(12)->isChecked())
		tweakSettings->setProperty< PropertyBoolean >(L"Runtime.DebugLayer", true);

	const int32_t language = m_toolLanguage->getSelected();
	if (language > 0)
		tweakSettings->setProperty< PropertyString >(L"Online.OverrideLanguageCode", c_languageCodes[language - 1].code);

	return tweakSettings;
}

void EditorPlugin::launch(TargetInstance* targetInstance)
{
	// Get selected target host.
	const int32_t id = targetInstance->getDeployHostId();
	if (id < 0)
		return;

	const std::wstring host = m_hostEnumerator->getHost(id);

	// Get our network host.
	std::wstring editorHost = L"localhost";
	net::SocketAddressIPv4::Interface itf;
	if (net::SocketAddressIPv4::getBestInterface(itf))
		editorHost = itf.addr->getHostName();
	else
		log::warning << L"Unable to determine editor host address; target might not be able to connect to editor database." << Endl;

	// Resolve absolute output path.
	const std::wstring outputPath = FileSystem::getInstance().getAbsolutePath(targetInstance->getOutputPath()).getPathName();

	// Set target's state to pending as actions can be queued up to be performed much later.
	targetInstance->setState(TsPending);
	targetInstance->setBuildProgress(0);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

		ActionChain chain;
		chain.targetInstance = targetInstance;

		Action action;

		// Create pipeline settings.
		Ref< PropertyGroup > pipelineSettings = new PropertyGroup();
		pipelineSettings->setProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions", { L"_DEBUG" });
		pipelineSettings->setProperty< PropertyBoolean >(L"Pipeline.EditorDeploy", true);

		// Create "tweak" settings.
		Ref< PropertyGroup > tweakSettings = getTweakSettings();

		// Add build output data action.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsBuilding);
		action.action = new BuildTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			pipelineSettings,
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			outputPath,
			tweakSettings,
			false
		);
		chain.actions.push_back(action);

		// Add deploy and launch actions.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsDeploying);
		action.action = new DeployTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			targetInstance->getName(),
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			editorHost,
			host,
			m_connectionManager->getListenPort(),
			targetInstance->getDatabaseName(),
			m_targetManager->getPort(),
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
			host,
			outputPath
		);
		chain.actions.push_back(action);

		m_targetActionQueue.push_back(chain);
		m_targetActionQueueSignal.set();
	}

	m_targetList->requestUpdate();
	m_lastLaunchedTargetInstance = targetInstance;
}

void EditorPlugin::eventTargetListBuild(TargetBuildEvent* event)
{
	TargetInstance* targetInstance = event->getInstance();

	// Resolve absolute output path.
	const std::wstring outputPath = FileSystem::getInstance().getAbsolutePath(targetInstance->getOutputPath()).getPathName();

	// Set target's state to pending as actions can be queued up to be performed much later.
	targetInstance->setState(TsPending);
	targetInstance->setBuildProgress(0);

	{
		T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_targetActionQueueLock);

		ActionChain chain;
		chain.targetInstance = targetInstance;

		Action action;

		// Create pipeline settings.
		Ref< PropertyGroup > pipelineSettings = new PropertyGroup();
		if ((event->getKeyState() & ui::KsControl) == 0)
			pipelineSettings->setProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions", { L"_DEBUG" });
		pipelineSettings->setProperty< PropertyBoolean >(L"Pipeline.EditorDeploy", true);

		// Create "tweak" settings.
		Ref< PropertyGroup > tweakSettings = getTweakSettings();

		// Add build output data action.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsBuilding);
		action.action = new BuildTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			pipelineSettings,
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			outputPath,
			tweakSettings,
			false
		);
		chain.actions.push_back(action);

		m_targetActionQueue.push_back(chain);
		m_targetActionQueueSignal.set();
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTargetListShowProfiler(TargetCaptureEvent* event)
{
	TargetInstance* targetInstance = event->getInstance();
	const int32_t connectionId = event->getConnectionIndex();

	RefArray< TargetConnection > connections = targetInstance->getConnections();
	if (connectionId >= 0 && connectionId < int32_t(connections.size()))
	{
		TargetConnection* connection = connections[connectionId];
		T_ASSERT(connection);

		Ref< ProfilerDialog > profilerDialog = new ProfilerDialog(connection);
		profilerDialog->create(m_parent);
		profilerDialog->show();
	}
}

void EditorPlugin::eventTargetListMigrate(TargetMigrateEvent* event)
{
	TargetInstance* targetInstance = event->getInstance();

	// Get selected target host.
	const int32_t id = targetInstance->getDeployHostId();
	if (id < 0)
		return;

	const std::wstring host = m_hostEnumerator->getHost(id);

	// Get our network host.
	std::wstring editorHost = L"localhost";
	net::SocketAddressIPv4::Interface itf;
	if (net::SocketAddressIPv4::getBestInterface(itf))
		editorHost = itf.addr->getHostName();
	else
		log::warning << L"Unable to determine editor host address; target might not be able to connect to editor database." << Endl;

	// Resolve absolute output path.
#if defined(_WIN32)
	std::wstring outputPath = FileSystem::getInstance().getAbsolutePath(targetInstance->getOutputPath()).getPathName();
#else
	std::wstring outputPath = FileSystem::getInstance().getAbsolutePath(targetInstance->getOutputPath()).getPathNameNoVolume();
#endif

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
			pipelineSettings->setProperty< PropertyStringSet >(L"ScriptPipeline.PreprocessorDefinitions", { L"_DEBUG" });

		// Also add property for pipelines to indicate we're launching through editor.
		pipelineSettings->setProperty< PropertyBoolean >(L"Pipeline.EditorDeploy", true);

		// Add build output data action.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsBuilding);
		action.action = new BuildTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			pipelineSettings,
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			outputPath,
			nullptr,
			false
		);
		chain.actions.push_back(action);

		// Add migrate actions.
		action.listener = new TargetInstanceProgressListener(m_targetList, targetInstance, TsMigrating);
		action.action = new MigrateTargetAction(
			m_editor->getSourceDatabase(),
			m_editor->getSettings(),
			targetInstance->getName(),
			targetInstance->getTarget(),
			targetInstance->getTargetConfiguration(),
			host,
			outputPath
		);
		chain.actions.push_back(action);

		m_targetActionQueue.push_back(chain);
		m_targetActionQueueSignal.set();
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTargetListPlay(TargetPlayEvent* event)
{
	TargetInstance* targetInstance = event->getInstance();
	if (targetInstance)
		launch(targetInstance);
}

void EditorPlugin::eventTargetListStop(TargetStopEvent* event)
{
	TargetInstance* targetInstance = event->getInstance();
	const int32_t connectionId = event->getConnectionIndex();

	RefArray< TargetConnection > connections = targetInstance->getConnections();
	if (connectionId >= 0 && connectionId < int32_t(connections.size()))
	{
		TargetConnection* connection = connections[connectionId];
		T_ASSERT(connection);

		connection->shutdown();
		targetInstance->removeConnection(connection);
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTargetListCommand(TargetCommandEvent* event)
{
	TargetInstance* targetInstance = event->getInstance();
	const int32_t connectionId = event->getConnectionIndex();

	RefArray< TargetConnection > connections = targetInstance->getConnections();
	if (connectionId >= 0 && connectionId < int32_t(connections.size()))
	{
		TargetConnection* connection = connections[connectionId];
		T_ASSERT(connection);

		CommandEvent commandEvent(event->getCommand());
		if (!connection->getTransport()->send(&commandEvent))
			log::error << L"Failed to issue command \"" << event->getCommand() << L"\" at target, unable to send command." << Endl;
	}
}

void EditorPlugin::eventToolBarClick(ui::ToolBarButtonClickEvent* event)
{
	const int32_t selectedTargetIndex = m_toolTargets->getSelected();
	if (selectedTargetIndex < 0 || selectedTargetIndex >= int32_t(m_targetInstances.size()))
		return;

	const EditTarget& et = m_targets[selectedTargetIndex];
	T_ASSERT(et.target);

	m_targetList->removeAll();

	for (auto targetInstance : m_targetInstances)
	{
		if (targetInstance->getTarget() == et.target)
			m_targetList->add(new TargetInstanceListItem(m_hostEnumerator, targetInstance));
	}

	m_targetList->requestUpdate();
}

void EditorPlugin::eventTimer(ui::TimerEvent* event)
{
	if (
		m_targetManager &&
		m_targetManager->update()
	)
	{
		// Doing both seems odd but deferred request is to ensure
		// widget layout also is updated properly.
		m_targetList->requestUpdate();
		m_targetList->update();
	}
}

void EditorPlugin::threadHostEnumerator()
{
	std::vector< std::wstring > localDeployPlatforms;
	while (!m_threadHostEnumerator->stopped())
	{
		m_hostEnumerator->update();

		// Find first local host.
		localDeployPlatforms.resize(0);
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
			for (auto targetInstance : m_targetInstances)
			{
				if (targetInstance->getDeployHostId() < 0)
				{
					const std::wstring platformName = targetInstance->getPlatformName();
					if (m_hostEnumerator->supportPlatform(localDeployHostId, platformName))
					{
						targetInstance->setDeployHostId(localDeployHostId);
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
		for (const auto& action : chain.actions)
		{
			success &= action.action->execute(action.listener);
			if (!success)
			{
				log::error << L"Deploy action \"" << type_name(action.action) << L"\" failed; unable to continue." << Endl;
				break;
			}
		}

		if (chain.targetInstance)
			chain.targetInstance->setState(TsIdle);

		m_targetList->requestUpdate();

		chain.actions.resize(0);
		chain.targetInstance = nullptr;
	}
}

}
