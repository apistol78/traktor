#include "Core/Log/Log.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/IProject.h"
#include "Editor/TypeBrowseFilter.h"
#include "Resource/ResourceManager.h"
#include "Sound/ISoundDriver.h"
#include "Sound/Sound.h"
#include "Sound/SoundFactory.h"
#include "Sound/SoundSystem.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/BankSound.h"
#include "Sound/Resound/IGrain.h"
#include "Sound/Resound/PlayGrain.h"
#include "Sound/Resound/RandomGrain.h"
#include "Sound/Resound/SequenceGrain.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Ui/Container.h"
#include "Ui/PopupMenu.h"
#include "Ui/MenuItem.h"
#include "Ui/MethodHandler.h"
#include "Ui/Slider.h"
#include "Ui/Static.h"
#include "Ui/TableLayout.h"
#include "Ui/Events/CommandEvent.h"
#include "Ui/Events/MouseEvent.h"
#include "Ui/Custom/QuadSplitter.h"
#include "Ui/Custom/GridView/GridColumn.h"
#include "Ui/Custom/GridView/GridItem.h"
#include "Ui/Custom/GridView/GridRow.h"
#include "Ui/Custom/GridView/GridView.h"
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

	Ref< ui::custom::QuadSplitter > splitter = new ui::custom::QuadSplitter();
	splitter->create(parent, ui::Point(50, 80), true);

	Ref< ui::Container > containerSounds = new ui::Container();
	containerSounds->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::custom::ToolBar > toolBarSounds = new ui::custom::ToolBar();
	toolBarSounds->create(containerSounds);
	toolBarSounds->addItem(new ui::custom::ToolBarButton(L"Play", ui::Command(L"Bank.PlaySound"), 0, ui::custom::ToolBarButton::BsText));
	toolBarSounds->addClickEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventToolBarClick));

	m_gridSounds = new ui::custom::GridView();
	m_gridSounds->create(containerSounds, ui::custom::GridView::WsColumnHeader | ui::WsClientBorder | ui::WsDoubleBuffer);
	m_gridSounds->addColumn(new ui::custom::GridColumn(L"Sounds", 0));
	m_gridSounds->addSelectEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGridSoundsSelect));
	m_gridSounds->addButtonUpEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGridSoundsButtonUp));

	Ref< ui::Container > containerGrains = new ui::Container();
	containerGrains->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	Ref< ui::custom::ToolBar > toolBarGrains = new ui::custom::ToolBar();
	toolBarGrains->create(containerGrains);
	toolBarGrains->addItem(new ui::custom::ToolBarButton(L"Play", ui::Command(L"Bank.PlayGrain"), 0, ui::custom::ToolBarButton::BsText));
	toolBarGrains->addClickEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventToolBarClick));

	m_gridGrains = new ui::custom::GridView();
	m_gridGrains->create(containerGrains, ui::custom::GridView::WsColumnHeader | ui::custom::GridView::WsDrag | ui::WsClientBorder | ui::WsDoubleBuffer);
	m_gridGrains->addColumn(new ui::custom::GridColumn(L"Grains", 0));
	m_gridGrains->addSelectEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGridGrainsSelect));
	m_gridGrains->addButtonUpEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGridGrainsButtonUp));

	m_containerGrainProperties = new ui::Container();
	m_containerGrainProperties->create(splitter, ui::WsClientBorder, new ui::TableLayout(L"100%", L"*,*", 0, 4));

	m_containerGrainProperties->setEnable(false);

	m_containerSoundProperties = new ui::Container();
	m_containerSoundProperties->create(splitter, ui::WsClientBorder, new ui::TableLayout(L"*,100%", L"*", 4, 4));

	Ref< ui::Static > staticPitch = new ui::Static();
	staticPitch->create(m_containerSoundProperties, L"Pitch");

	m_sliderPitch = new ui::Slider();
	m_sliderPitch->create(m_containerSoundProperties);
	m_sliderPitch->addChangeEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventSliderPitchChange));
	m_sliderPitch->setRange(50, 200);
	m_sliderPitch->setValue(100);

	m_containerSoundProperties->setEnable(false);

	m_menuGrains = new ui::PopupMenu();
	m_menuGrains->create();
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.AddGrain"), L"Add grain..."));
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.RemoveSound"), L"Remove grain..."));

	m_menuSounds = new ui::PopupMenu();
	m_menuSounds->create();
	m_menuSounds->add(new ui::MenuItem(ui::Command(L"Bank.AddSound"), L"Add sound..."));
	m_menuSounds->add(new ui::MenuItem(ui::Command(L"Bank.RemoveSound"), L"Remove sound..."));

	updateGridGrains();
	updateGridSounds();

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
				m_editor->getProject()->getOutputDatabase(),
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
	if (m_soundSystem)
	{
		m_soundSystem->destroy();
		m_soundSystem = 0;
	}
	if (m_menuSounds)
	{
		m_menuSounds->destroy();
		m_menuSounds = 0;
	}
	if (m_menuGrains)
	{
		m_menuGrains->destroy();
		m_menuGrains = 0;
	}
}

void BankAssetEditor::apply()
{
}

void BankAssetEditor::updateGridGrains(ui::custom::GridRow* parentRow, const RefArray< IGrain >& grains)
{
	for (RefArray< IGrain >::const_iterator i = grains.begin(); i != grains.end(); ++i)
	{
		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->addItem(new ui::custom::GridItem(type_name(*i)));
		row->setData(L"GRAIN", *i);

		if (parentRow)
			parentRow->addChild(row);
		else
			m_gridGrains->addRow(row);

		if (const RandomGrain* randomGrain = dynamic_type_cast< const RandomGrain* >(*i))
		{
			const RefArray< IGrain >& childGrains = randomGrain->getGrains();
			updateGridGrains(row, childGrains);
		}
		else if (const SequenceGrain* sequenceGrain = dynamic_type_cast< const SequenceGrain* >(*i))
		{
			const RefArray< IGrain >& childGrains = sequenceGrain->getGrains();
			updateGridGrains(row, childGrains);
		}
	}
}

void BankAssetEditor::updateGridGrains()
{
	m_gridGrains->removeAllRows();

	const RefArray< IGrain >& grains = m_asset->getGrains();
	updateGridGrains(0, grains);

	m_gridGrains->update();
}

void BankAssetEditor::updateGridSounds()
{
	m_gridSounds->removeAllRows();

	const RefArray< BankSound >& sounds = m_asset->getSounds();
	for (RefArray< BankSound >::const_iterator i = sounds.begin(); i != sounds.end(); ++i)
	{
		Guid soundGuid = (*i)->getSound().getGuid();

		Ref< db::Instance > soundAssetInstance = m_editor->getProject()->getSourceDatabase()->getInstance(soundGuid);
		std::wstring soundName = soundAssetInstance ? soundAssetInstance->getName() : soundGuid.format();

		Ref< ui::custom::GridRow > row = new ui::custom::GridRow();
		row->addItem(new ui::custom::GridItem(soundName));
		row->setData(L"SOUND", *i);
		m_gridSounds->addRow(row);
	}

	m_gridSounds->update();
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
			updateGridGrains();
		}
	}
	else if (command == L"Bank.AddSound")
	{
		TypeInfoSet typeSet;
		typeSet.insert(&type_of< BankAsset >());
		typeSet.insert(&type_of< SoundAsset >());
		editor::TypeBrowseFilter filter(typeSet);

		Ref< db::Instance > instance = m_editor->browseInstance(&filter);
		if (instance)
		{
			Ref< BankSound > sound = new BankSound(
				instance->getGuid(),
				1.0f
			);
			m_asset->addSound(sound);
			updateGridSounds();
		}
	}
	else if (command == L"Bank.PlayGrain")
	{
		Ref< ui::custom::GridRow > selectedRow = m_gridGrains->getSelectedRow();
		if (selectedRow)
		{
			IGrain* grain = selectedRow->getData< IGrain >(L"GRAIN");
			T_ASSERT (grain);

			const RefArray< BankSound >& sounds = m_asset->getSounds();
			for (RefArray< BankSound >::const_iterator i = sounds.begin(); i != sounds.end(); ++i)
				(*i)->bind(m_resourceManager);

			RefArray< IGrain > grains;
			grains.push_back(grain);

			Ref< BankBuffer > bankBuffer = new BankBuffer(grains, sounds);
			Ref< Sound > sound = new Sound(bankBuffer);

			m_soundSystem->play(
				sound,
				true
			);
		}
	}
	else if (command == L"Bank.PlaySound")
	{
		Ref< ui::custom::GridRow > selectedRow = m_gridSounds->getSelectedRow();
		if (selectedRow)
		{
			BankSound* bankSound = selectedRow->getData< BankSound >(L"SOUND");
			T_ASSERT (bankSound);

			bankSound->bind(m_resourceManager);

			RefArray< IGrain > grains;
			grains.push_back(new PlayGrain(0));

			RefArray< BankSound > sounds;
			sounds.push_back(bankSound);

			Ref< BankBuffer > bankBuffer = new BankBuffer(grains, sounds);
			Ref< Sound > sound = new Sound(bankBuffer);

			m_soundSystem->play(
				sound,
				true
			);
		}
	}
}

void BankAssetEditor::eventToolBarClick(ui::Event* event)
{
	ui::CommandEvent* commandEvent = checked_type_cast< ui::CommandEvent* >(event);
	handleCommand(commandEvent->getCommand());
}

void BankAssetEditor::eventGridGrainsSelect(ui::Event* event)
{
	Ref< ui::custom::GridRow > selectedRow = m_gridGrains->getSelectedRow();
	if (selectedRow)
	{
		Ref< IGrain > grain = selectedRow->getData< IGrain >(L"GRAIN");
		T_ASSERT (grain);

		// @fixme
	}
}

void BankAssetEditor::eventGridGrainsButtonUp(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_menuGrains->show(m_gridGrains, mouseEvent->getPosition());
		if (selectedItem)
			handleCommand(selectedItem->getCommand());
		mouseEvent->consume();
	}
}

void BankAssetEditor::eventGridSoundsSelect(ui::Event* event)
{
	Ref< ui::custom::GridRow > selectedRow = m_gridSounds->getSelectedRow();
	if (selectedRow)
	{
		Ref< BankSound > sound = selectedRow->getData< BankSound >(L"SOUND");
		T_ASSERT (sound);

		m_sliderPitch->setValue(int32_t(sound->getPitch() * 100.0f));

		m_containerSoundProperties->setEnable(true);
	}
	else
		m_containerSoundProperties->setEnable(false);
}

void BankAssetEditor::eventGridSoundsButtonUp(ui::Event* event)
{
	ui::MouseEvent* mouseEvent = checked_type_cast< ui::MouseEvent* >(event);
	if (mouseEvent->getButton() == ui::MouseEvent::BtRight)
	{
		Ref< ui::MenuItem > selectedItem = m_menuSounds->show(m_gridSounds, mouseEvent->getPosition());
		if (selectedItem)
			handleCommand(selectedItem->getCommand());
		mouseEvent->consume();
	}
}

void BankAssetEditor::eventSliderPitchChange(ui::Event* event)
{
	Ref< ui::custom::GridRow > selectedRow = m_gridSounds->getSelectedRow();
	if (selectedRow)
	{
		Ref< BankSound > sound = selectedRow->getData< BankSound >(L"SOUND");
		T_ASSERT (sound);

		float pitch = m_sliderPitch->getValue() / 100.0f;
		sound->setPitch(pitch);
	}
}

	}
}
