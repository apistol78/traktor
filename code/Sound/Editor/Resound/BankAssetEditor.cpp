#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Resource/ResourceManager.h"
#include "Sound/ISoundDriver.h"
#include "Sound/Sound.h"
#include "Sound/SoundFactory.h"
#include "Sound/SoundSystem.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Resound/RandomGrain.h"
#include "Sound/Resound/RepeatGrain.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/DefaultGrainEditor.h"
#include "Sound/Editor/Resound/GrainView.h"
#include "Sound/Editor/Resound/GrainViewItem.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/Splitter.h"
#include "Ui/Custom/ToolBar/ToolBar.h"
#include "Ui/Custom/ToolBar/ToolBarButton.h"

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.BankAssetEditor", BankAssetEditor, editor::IObjectEditor)

BankAssetEditor::BankAssetEditor(editor::IEditor* editor)
:	m_editor(editor)
{
}

bool BankAssetEditor::create(ui::Widget* parent, db::Instance* instance, ISerializable* object)
{
	m_instance = instance;
	if (!m_instance)
		return false;

	m_asset = dynamic_type_cast< BankAsset* >(object);
	if (!m_asset)
		return false;

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, 30, true);

	Ref< ui::Container > containerGrains = new ui::Container();
	containerGrains->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::custom::ToolBar > toolBarGrains = new ui::custom::ToolBar();
	toolBarGrains->create(containerGrains);
	toolBarGrains->addItem(new ui::custom::ToolBarButton(L"Play", ui::Command(L"Bank.PlayGrain"), 0, ui::custom::ToolBarButton::BsText));
	toolBarGrains->addClickEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventToolBarClick));

	m_grainView = new GrainView();
	m_grainView->create(containerGrains);
	m_grainView->addEventHandler(ui::EiSelectionChange, ui::createMethodHandler(this, &BankAssetEditor::eventGrainSelect));
	m_grainView->addButtonUpEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGrainButtonUp));

	m_containerGrainProperties = new ui::Container();
	m_containerGrainProperties->create(splitter, ui::WsClientBorder, new ui::TableLayout(L"100%", L"100%", 0, 4));

	m_menuGrains = new ui::PopupMenu();
	m_menuGrains->create();
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.AddGrain"), L"Add grain..."));
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.RemoveSound"), L"Remove grain..."));

	updateGrainView();

	// Create grain editors.
	m_grainEditorDefault = new DefaultGrainEditor(m_editor);
	m_grainEditorDefault->create(m_containerGrainProperties);

	// Create sound system used for preview.
	const TypeInfo* soundDriverType = TypeInfo::find(L"traktor.sound.SoundDriverDs8");
	if (soundDriverType)
	{
		Ref< sound::ISoundDriver > soundDriver = checked_type_cast< ISoundDriver*, false >(soundDriverType->createInstance());

		m_soundSystem = new sound::SoundSystem(soundDriver);

		sound::SoundSystemCreateDesc sscd;
		sscd.channels = 4;
		sscd.driverDesc.sampleRate = 44100;
		sscd.driverDesc.bitsPerSample = 16;
		sscd.driverDesc.hwChannels = 2;
		sscd.driverDesc.frameSamples = 1400;

		if (!m_soundSystem->create(sscd))
			m_soundSystem = 0;


		m_resourceManager = new resource::ResourceManager();
		m_resourceManager->addFactory(
			new SoundFactory(
				m_editor->getOutputDatabase(),
				m_soundSystem
			)
		);
	}

	if (!m_soundSystem)
		log::warning << L"Unable to create preview sound system; preview unavailable" << Endl;

	return true;
}

void BankAssetEditor::destroy()
{
	if (m_resourceManager)
		m_resourceManager = 0;

	safeDestroy(m_soundSystem);
	safeDestroy(m_menuGrains);
	safeDestroy(m_grainEditorDefault);
}

void BankAssetEditor::apply()
{
}

void BankAssetEditor::updateGrainView(GrainViewItem* parent, const RefArray< IGrain >& grains)
{
	for (RefArray< IGrain >::const_iterator i = grains.begin(); i != grains.end(); ++i)
	{
		Ref< GrainViewItem > item = new GrainViewItem(parent, *i);
		m_grainView->add(item);

		if (const RandomGrain* randomGrain = dynamic_type_cast< const RandomGrain* >(*i))
		{
			const RefArray< IGrain >& childGrains = randomGrain->getGrains();
			updateGrainView(item, childGrains);
		}
		else if (const RepeatGrain* repeatGrain = dynamic_type_cast< const RepeatGrain* >(*i))
		{
			RefArray< IGrain > childGrains; childGrains.push_back(repeatGrain->getGrain());
			updateGrainView(item, childGrains);
		}
		else if (const SequenceGrain* sequenceGrain = dynamic_type_cast< const SequenceGrain* >(*i))
		{
			const RefArray< IGrain >& childGrains = sequenceGrain->getGrains();
			updateGrainView(item, childGrains);
		}
	}
}

void BankAssetEditor::updateGrainView()
{
	m_grainView->removeAll();

	const RefArray< IGrain >& grains = m_asset->getGrains();
	updateGrainView(0, grains);

	m_grainView->update();
}

void BankAssetEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Bank.AddGrain")
	{
		const TypeInfo* grainType = m_editor->browseType(&type_of< IGrain >());
		if (grainType)
		{
			Ref< IGrain > grain = checked_type_cast< IGrain*, false >(grainType->createInstance());
			m_asset->addGrain(grain);
			updateGrainView();
		}
	}
	else if (command == L"Bank.PlayGrain")
	{
		RefArray< IGrain > grains;

		// Play only selected grain.
		GrainViewItem* selectedItem = m_grainView->getSelected();
		if (selectedItem)
		{
			IGrain* grain = selectedItem->getGrain();
			T_ASSERT (grain);

			grains.push_back(grain);
		}
		else
		{
			grains = m_asset->getGrains();
		}

		for (RefArray< IGrain >::iterator i = grains.begin(); i != grains.end(); ++i)
			(*i)->bind(m_resourceManager);

		Ref< BankBuffer > bankBuffer = new BankBuffer(grains);
		Ref< Sound > sound = new Sound(bankBuffer);

		m_soundSystem->play(
			sound,
			true
		);
	}
}

void BankAssetEditor::eventToolBarClick(ui::Event* event)
{
	ui::CommandEvent* commandEvent = checked_type_cast< ui::CommandEvent*, false >(event);
	handleCommand(commandEvent->getCommand());
}

void BankAssetEditor::eventGrainSelect(ui::Event* event)
{
	GrainViewItem* item = checked_type_cast< GrainViewItem* >(event->getItem());
	if (item)
	{
		m_grainEditorDefault->hide();
		m_grainEditorDefault->show(item->getGrain());
	}
	else
		m_grainEditorDefault->hide();
}

void BankAssetEditor::eventGrainButtonUp(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent*, false >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_menuGrains->show(m_grainView, mouseEvent->getPosition());
		if (selectedItem)
			handleCommand(selectedItem->getCommand());
		mouseEvent->consume();
	}
}

	}
}
