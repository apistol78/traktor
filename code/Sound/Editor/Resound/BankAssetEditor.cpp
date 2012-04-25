#include "Core/Log/Log.h"
#include "Core/Misc/SafeDestroy.h"
#include "Database/Database.h"
#include "Database/Instance.h"
#include "Editor/IEditor.h"
#include "Editor/TypeBrowseFilter.h"
#include "Resource/ResourceManager.h"
#include "Sound/Sound.h"
#include "Sound/SoundChannel.h"
#include "Sound/SoundFactory.h"
#include "Sound/SoundSystem.h"
#include "Sound/Resound/BankBuffer.h"
#include "Sound/Resound/MuteGrainData.h"
#include "Sound/Resound/PlayGrainData.h"
#include "Sound/Resound/RandomGrainData.h"
#include "Sound/Resound/RepeatGrainData.h"
#include "Sound/Resound/SequenceGrainData.h"
#include "Sound/Editor/SoundAsset.h"
#include "Sound/Editor/SoundSystemFactory.h"
#include "Sound/Editor/Resound/BankAsset.h"
#include "Sound/Editor/Resound/BankAssetEditor.h"
#include "Sound/Editor/Resound/GrainProperties.h"
#include "Sound/Editor/Resound/GrainView.h"
#include "Sound/Editor/Resound/GrainViewItem.h"
#include "Sound/Editor/Resound/MuteGrainFacade.h"
#include "Sound/Editor/Resound/PlayGrainFacade.h"
#include "Sound/Editor/Resound/RandomGrainFacade.h"
#include "Sound/Editor/Resound/RepeatGrainFacade.h"
#include "Sound/Editor/Resound/SequenceGrainFacade.h"
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

	parent->addTimerEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventTimer));
	parent->startTimer(100);

	Ref< ui::custom::Splitter > splitter = new ui::custom::Splitter();
	splitter->create(parent, true, 30, true);

	Ref< ui::Container > containerGrains = new ui::Container();
	containerGrains->create(splitter, ui::WsNone, new ui::TableLayout(L"100%", L"*,100%", 0, 0));

	m_toolBarItemPlay = new ui::custom::ToolBarButton(L"Play", ui::Command(L"Bank.PlayGrain"), 0, ui::custom::ToolBarButton::BsText | ui::custom::ToolBarButton::BsToggle);

	m_toolBar = new ui::custom::ToolBar();
	m_toolBar->create(containerGrains);
	m_toolBar->addItem(m_toolBarItemPlay);
	m_toolBar->addClickEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventToolBarClick));

	m_grainView = new GrainView();
	m_grainView->create(containerGrains);
	m_grainView->addEventHandler(ui::EiSelectionChange, ui::createMethodHandler(this, &BankAssetEditor::eventGrainSelect));
	m_grainView->addButtonUpEventHandler(ui::createMethodHandler(this, &BankAssetEditor::eventGrainButtonUp));

	m_containerGrainProperties = new ui::Container();
	m_containerGrainProperties->create(splitter, ui::WsClientBorder, new ui::TableLayout(L"100%", L"100%", 0, 4));

	m_grainProperties = new GrainProperties(m_editor);
	m_grainProperties->create(m_containerGrainProperties);
	m_grainProperties->addEventHandler(ui::EiUser + 1, ui::createMethodHandler(this, &BankAssetEditor::eventGrainPropertiesChange));

	m_menuGrains = new ui::PopupMenu();
	m_menuGrains->create();
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.AddGrain"), L"Add grain..."));
	m_menuGrains->add(new ui::MenuItem(ui::Command(L"Bank.RemoveGrain"), L"Remove grain..."));

	// Create grain editor facades.
	m_grainFacades[&type_of< MuteGrainData >()] = new MuteGrainFacade();
	m_grainFacades[&type_of< PlayGrainData >()] = new PlayGrainFacade();
	m_grainFacades[&type_of< RandomGrainData >()] = new RandomGrainFacade();
	m_grainFacades[&type_of< RepeatGrainData >()] = new RepeatGrainFacade();
	m_grainFacades[&type_of< SequenceGrainData >()] = new SequenceGrainFacade();

	// Get sound system for preview.
	Ref< SoundSystemFactory > soundSystemFactory = m_editor->getStoreObject< SoundSystemFactory >(L"SoundSystemFactory");
	if (soundSystemFactory)
	{
		m_soundSystem = soundSystemFactory->createSoundSystem();
		if (!m_soundSystem)
			log::warning << L"Unable to create preview sound system; preview unavailable" << Endl;
	}

	m_resourceManager = new resource::ResourceManager();
	m_resourceManager->addFactory(new SoundFactory(
		m_editor->getOutputDatabase()
	));

	updateGrainView();
	return true;
}

void BankAssetEditor::destroy()
{
	if (m_soundChannel)
	{
		m_soundChannel->stop();
		m_soundChannel = 0;
	}

	if (m_resourceManager)
		m_resourceManager = 0;

	safeDestroy(m_soundSystem);
	safeDestroy(m_menuGrains);
}

void BankAssetEditor::apply()
{
	m_instance->setObject(m_asset);
}

ui::Size BankAssetEditor::getPreferredSize() const
{
	return ui::Size(500, 400);
}

void BankAssetEditor::updateGrainView(GrainViewItem* parent, const RefArray< IGrainData >& grains)
{
	for (RefArray< IGrainData >::const_iterator i = grains.begin(); i != grains.end(); ++i)
	{
		IGrainFacade* grainFacade = m_grainFacades[&type_of(*i)];
		if (!grainFacade)
			continue;

		Ref< GrainViewItem > item = new GrainViewItem(
			parent,
			*i,
			grainFacade->getText(*i),
			grainFacade->getImage(*i)
		);
		m_grainView->add(item);

		RefArray< IGrainData > childGrains;
		if (grainFacade->getChildren(*i, childGrains))
			updateGrainView(item, childGrains);
	}
}

void BankAssetEditor::updateGrainView()
{
	m_grainView->removeAll();

	const RefArray< IGrainData >& grains = m_asset->getGrains();
	updateGrainView(0, grains);

	m_grainView->update();
}

void BankAssetEditor::handleCommand(const ui::Command& command)
{
	if (command == L"Bank.AddGrain")
	{
		IGrainData* parentGrain = 0;
		IGrainFacade* parentGrainFacade = 0;

		GrainViewItem* selectedItem = m_grainView->getSelected();
		if (selectedItem)
		{
			parentGrain = selectedItem->getGrain();
			T_ASSERT (parentGrain);

			parentGrainFacade = m_grainFacades[&type_of(parentGrain)];
			if (!parentGrainFacade || !parentGrainFacade->canHaveChildren())
				return;
		}

		const TypeInfo* grainType = m_editor->browseType(&type_of< IGrainData >());
		if (grainType)
		{
			Ref< IGrainData > grain = checked_type_cast< IGrainData*, false >(grainType->createInstance());

			if (parentGrainFacade)
				parentGrainFacade->addChild(parentGrain, grain);
			else
				m_asset->addGrain(grain);

			updateGrainView();
		}
	}
	else if (command == L"Bank.RemoveGrain")
	{
		GrainViewItem* selectedItem = m_grainView->getSelected();
		if (selectedItem)
		{
			IGrainData* grain = selectedItem->getGrain();
			T_ASSERT (grain);

			if (selectedItem->getParent())
			{
				IGrainData* parentGrain = selectedItem->getParent()->getGrain();
				T_ASSERT (parentGrain);

				IGrainFacade* parentGrainFacade = m_grainFacades[&type_of(parentGrain)];
				if (!parentGrainFacade || !parentGrainFacade->canHaveChildren())
					return;

				parentGrainFacade->removeChild(parentGrain, grain);
			}
			else
				m_asset->removeGrain(grain);

			updateGrainView();
		}
	}
	else if (command == L"Bank.PlayGrain")
	{
		if (!m_soundChannel)
		{
			RefArray< IGrainData > grainData;
			RefArray< IGrain > grains;

			// Play only selected grain.
			GrainViewItem* selectedItem = m_grainView->getSelected();
			if (selectedItem)
			{
				IGrainData* grain = selectedItem->getGrain();
				T_ASSERT (grain);

				grainData.push_back(grain);
			}
			else
			{
				grainData = m_asset->getGrains();
			}

			// Create grains from data.
			grains.resize(grainData.size());
			for (uint32_t i = 0; i < grainData.size(); ++i)
				grains[i] = grainData[i]->createInstance(m_resourceManager);

			m_bankBuffer = new BankBuffer(grains);
			m_soundChannel = m_soundSystem->play(
				new Sound(m_bankBuffer),
				0,
				true
			);

			if (m_soundChannel)
			{
				m_toolBarItemPlay->setToggled(true);
				m_toolBar->update();
			}
		}
		else
		{
			m_soundChannel->stop();
			m_soundChannel = 0;

			m_toolBarItemPlay->setToggled(false);
			m_toolBar->update();
		}
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
		m_grainProperties->set(item->getGrain());
	else
		m_grainProperties->set(0);
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

void BankAssetEditor::eventGrainPropertiesChange(ui::Event* event)
{
	// Stop playing if properties has changed, need to reflect changes
	// without interference otherwise filter instances will be incorrect.
	if (m_soundChannel && m_bankBuffer)
	{
		ISoundBufferCursor* cursor = m_soundChannel->getCursor();
		if (cursor)
			m_bankBuffer->updateCursor(cursor);
	}

	updateGrainView();
}

void BankAssetEditor::eventTimer(ui::Event* event)
{
	if (!m_soundChannel)
		return;

	if (!m_soundChannel->isPlaying())
	{
		m_soundChannel = 0;

		m_toolBarItemPlay->setToggled(false);
		m_toolBar->update();
	}
}

	}
}
